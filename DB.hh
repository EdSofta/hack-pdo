<?hh // strict
namespace Schlunix\Pdo;
/**
 *  DB - A simple database class
 *
 * @author        Author: Vivek Wicky Aswal. (https://twitter.com/#!/VivekWickyAswal)
 * @git         https://github.com/indieteq/PHP-MySQL-PDO-Database-Class
 * @version      0.2ab
 *
 */
require("Log.hh");

class DB<T>
{
    # @object, PDO object
    # Must be nullable because CloseConnection sets it to null
    private ?\PDO $pdo;

    # @object, PDO statement object
    private ?\PDOStatement $sQuery;

    # @bool,  Connected to the database
    private bool $isConnected = false;

    # @object, Object for logging exceptions
    private Log $log;

    # @bool, executed query successfully
    private bool $success;

    # @array, The parameters of the SQL query
    private Vector<Map<string, string>> $parameters;

    private string $INIFileLocation;
    /**
    *   Default Constructor
    *
    *    1. Instantiate Log class.
    *    2. Connect to database.
    *    3. Creates the parameter array.
    */
    public function __construct()
    {
        $this->log = new Log();
        //$this->Connect();
        $this->parameters = Vector{};
        $this->sQuery     = null;
        $this->success    = false;
        $this->INIFileLocation = "";
    } // end constructor

    /**
    *    This method sets the location of the INI file to read DB settings from.
    *
    *    @param string $in <location of the INI file that contains database settings>
    */
    public function setConfigFile(string $in): void
    {
        $this->INIFileLocation = $in;
    }
    
    /**
    *    This method makes connection to the database.
    *
    *    1. Reads the database settings from a ini file.
    *    2. Puts  the ini content into the settings array.
    *    3. Tries to connect to the database.
    *    4. If connection failed, exception is displayed and a log file gets created.
    */
    public function Connect(string $dbName = "-1"): void
    {
        // if no dbName was given, default to the only section in the INI file,
        // otherwise we will be looking for the specific dbName
        $iniSettingsArray = parse_ini_file($this->INIFileLocation, true);
        
        if ($dbName === "-1") {
            // default settings, use the first section of the INI file 
            // get the first key in the associative array 
            $keys = array_keys($iniSettingsArray);
            
            // set iniSettings to the array that is the first section of the INI file 
            $iniSettings = $iniSettingsArray[$keys[0]];
        } else {
            // get the db settings for the specific db name 
            $iniSettings = $iniSettingsArray[$dbName];
        }
        
        $dsn = 'mysql:dbname=' . $iniSettings["dbname"] . ';host=' . $iniSettings["host"].'';
        try
        {
            # Read settings from INI file, set UTF8
            $local_pdo = new \PDO($dsn,
                                  $iniSettings["user"],
                                  $iniSettings["password"],
                                  array(\PDO::MYSQL_ATTR_INIT_COMMAND => "SET NAMES utf8"));

            # We can now log any exceptions on Fatal error.
            $local_pdo->setAttribute(\PDO::ATTR_ERRMODE, \PDO::ERRMODE_EXCEPTION);

            # Disable emulation of prepared statements, use REAL prepared statements instead.
            $local_pdo->setAttribute(\PDO::ATTR_EMULATE_PREPARES, false);

            // set class variable $pdo to the $local_pdo
            $this->pdo = $local_pdo;

            # Connection succeeded, set the boolean to true.
            $this->isConnected = true;
        }
        catch (\PDOException $e)
        {
            # Write into log
            echo $this->ExceptionLog($e->getMessage());
            die();
        }
    } // end function

    
    /*
    *   You can use this little method if you want to close the PDO connection
    *
    */
    public function CloseConnection(): void
    {
        # Set the PDO object to null to close the connection
        # http://www.php.net/manual/en/pdo.connections.php
        $this->pdo = null;
    } // end function

    
    /**
    *    Every method which needs to execute a SQL query uses this method.
    *
    *    1. If not connected, connect to the database.
    *    2. Prepare Query.
    *    3. Parameterize Query.
    *    4. Execute Query.
    *    5. On exception : Write Exception into the log + SQL query.
    *    6. Reset the Parameters.
    */
    private function Init(string $query, Vector<Vector<string>> $parameters = Vector{}): void
    {
        # Connect to database
        if (!$this->isConnected) {
            $this->Connect();
        }
        // get local copy of $this->pdo to test safely for null
        // we can't proceed unless there is a good pdo object available
        $local_pdo = $this->pdo;
        if (!is_null($local_pdo)) {
            try {
                # Prepare query
                $local_sQuery = $local_pdo->prepare($query);

                # Add parameters to the parameter array
                $this->setBindParameters($parameters);

                # Bind parameters
                if (count($this->parameters) > 0) {
                    foreach ($this->parameters as $parameterMap) {
                        $id = $parameterMap["bindId"];
                        $param = $parameterMap["bindValue"];
                        if (count($parameterMap) === 3) {
                            // only need type if we're in here 
                            $type = constant($parameterMap["bindType"]);
                            
                            // 3 parameters received, include a bind type
                            $local_sQuery->bindParam($id, $param, $type);
                        } elseif (count($parameterMap) === 2) {
                            
                            // 2 parameters received, don't include a bind type 
                            $local_sQuery->bindParam($id, $param);
                        } else {
                            // nothing to do, should never end up here, but add some logging 
                        }
                    } // end for each
                } // end if there are parameters

                # Execute SQL
                $this->success = $local_sQuery->execute();

                // set the class variable to the local instance
                $this->sQuery = $local_sQuery;
            } // end try block
            catch(\PDOException $e)
            {
                    # Write into log and display Exception
                    echo $this->ExceptionLog($e->getMessage(), $query );
                    die();
            } // end catch block
        } // end if local_pdo is not null

        # Reset the parameters property 
        $this->parameters = Vector{};
    } // end function


    /**
    *    @void
    *
    *    Add more parameters to the parameter array
    *    @param array $vector_bindParameters
    */
    public function setBindParameters(Vector<Vector<string>> $vector_bindParameters): void
    {
        // if there is nothing in the class $parameters property, and we received an
        // array of parameters, continue processing 
        if((count($this->parameters) == 0) && (count($vector_bindParameters) > 0)) {
            // add each bind parameter given my the caller to the class property $parameters 
            foreach($vector_bindParameters as $vector_params) {
                
                if (count($vector_params) === 3) {
                    // 3 parameters received, include a bind type 
                    $this->parameters[] = Map{"bindId"    => ":" . $vector_params[0],
                                              "bindValue" => $vector_params[1],
                                              "bindType"  => $vector_params[2]};
                } elseif (count($vector_params) === 2) {
                    // 2 parameters received, don't include a bind type 
                    $this->parameters[] = Map{"bindId"    => ":" . $vector_params[0],
                                              "bindValue" => $vector_params[1]};
                } else {
                    // illegal number of parameters received, do nothing here 
                    // add some logging later on 
                    return;
                }
            } // end foreach

        } // end if
    } // end function
    
    
    /**
    * If the SQL query  contains a SELECT or SHOW statement it returns an array
    * containing all of the result set row
    *    If the SQL statement is a DELETE, INSERT, or UPDATE statement it returns
    * the number of affected rows
    *
    *    @param  string $query
    *    @param  Vector(of Vector<string>)  $params
    *    @param  int    $fetchmode
    *    @return mixed
    */
    public function query(string $query,
                          Vector<Vector<string>> $params = Vector{},
                          int $fetchmode = \PDO::FETCH_ASSOC): mixed
    {
        $query = trim($query);

        $this->Init($query, $params);

        $rawStatement = explode(" ", $query);

        # Which SQL statement is used
        $statement = strtolower($rawStatement[0]);

        // get a local copy of sQuery to test for null
        $local_sQuery = $this->sQuery;

        // test to see if $local_sQuery is null, if it is, there's nothing to do
        if (!is_null($local_sQuery)) {
            // determine the query type
            if ($statement === 'select' || $statement === 'show') {
                return $local_sQuery->fetchAll($fetchmode);
                
            } elseif ( $statement    === 'insert'
                       || $statement === 'update'
                       || $statement === 'delete' ) {
                return $local_sQuery->rowCount();
                
            } else {
                // no query type, so return null to get out
                return null;
            }
        } else {
            // $local_sQuery was null, so nothing to do, return null
            return null;
        } // end if $local_sQuery is not null
    } // end function

    
    /**
    *  Returns the last inserted id.
    *  @return string
    */
    public function lastInsertId(): ?string
    {
        // get local copy of $this->pdo to test safely for null
        // we can't proceed unless there is a good pdo object available
        $local_pdo = $this->pdo;
        if (!is_null($local_pdo)) {
            return $local_pdo->lastInsertId();
        } else {
            // if there is no pdo session available (this should never be the case)
            // return null
            return null;
        } // end if
    } // end function

    
    /**
    *    Returns an array which represents a column from the result set
    *
    *    @param  string $query
    *    @param  array  $params
    *    @return array
    */
    public function column(string $query,
                           Vector<Vector<string>> $params = Vector{}): ?Vector<string>
    {
        $this->Init($query, $params);

        // get local copy of sQuery to test for null
        $local_sQuery = $this->sQuery;
        if (!is_null($local_sQuery)) {
            $Columns = $local_sQuery->fetchAll(\PDO::FETCH_NUM);

            $column = Vector{};

            foreach($Columns as $cells) {
                $column[] = $cells[0];
            } // end foreach

            return $column;
        } else {
            // if there was no query to execute, return null
            return null;
        }
    } // end function

    
    /**
    * Writes the log and returns the exception
    *
    * @param  string $message
    * @param  string $sql
    * @return string
    */
    private function ExceptionLog(string $message , ?string $sql = null): string
    {
        $exception  = "Unhandled Exception. <br /> $message <br /> You can find the error back in the log.";

        if (!is_null($sql)) {
            # Add the Raw SQL to the Log
            $message .= "\r\nRaw SQL : "  . $sql;
        }
            # Write into log
            $this->log->write($message);

        return $exception;
    } // end function

} // end class