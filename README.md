PDO Database Class
============================

A database class for Hack/HHVM-MySQL which uses the PDO extension.

## To use the class
#### 1. Edit the database settings in the settings.ini.php
##### You can include as many database settings groups as you want in this file as long as they each have a unique name.

    [SQL]
    host = 127.0.0.1
    user = root
    password = secret
    dbname = yourdatabase

#### 2. Require the class in your project
I highly recommend using a PSR-4 autoloader. There is a Composer file included in this project for just such a case.
The DB class is written in hack strict, so it can be included without errors in other strict code

    <?hh // strict
    require("DB.hh");
    
If you are autoloading, include the namespace and class, no require needed

    <?hh // strict
    use Schlunix\Pdo\DB;

#### 3. Create a new DB object
    <?hh // strict
    // Instantiate the class
    $db = new DB();

#### 4.  Logs

This project's logging functionality is built around Klogger. If your calling script already has a Klogger object you can simply pass it to the DB class using:
    
    $db->setLogger(yourLoggerObject);

If you do not already have a Klogger object from the calling script, you can create one in the DB class using:

    $db->setLogLocation(String logLocation, String logLevel, array<Klogger options>);

## Examples
Below some examples of the basic functions of the database class.
#### The persons table 

    | id | firstname   | lastname    | sex | age |
    |:--:|:-----------:|:-----------:|:---:|:---:|
    | 1  |    John     |     Doe     | M   | 19  |
    | 2  |    Bob      |     Black   | M   | 41  |
    | 3  |    Zoe      |     Chan    | F   | 20  |
    | 4  |    Kona     |     Khan    | M   | 14  |
    | 5  |    Kader    |     Khan    | M   | 56  |

#### Use normal SQL queries

    <?hh // strict
    // Instantiate the class
    $db = new DB();
    // Fetch whole table
    $persons = $db->query("SELECT * FROM persons");

#### Fetching with Bindings (ANTI-SQL-INJECTION):
Binding parameters is the best way to prevent SQL injection. The class prepares your SQL query and binds the parameters
afterwards.

There are three different ways to bind parameters.
```php
<?php
// 1. Read friendly method  
$db->bind("id","1");
$db->bind("firstname","John");
$person   =  $db->query("SELECT * FROM Persons WHERE firstname = :firstname AND id = :id");

// 2. Bind more parameters
$db->bindMore(array("firstname"=>"John","id"=>"1"));
$person   =  $db->query("SELECT * FROM Persons WHERE firstname = :firstname AND id = :id"));

// 3. Or just give the parameters to the method
$person   =  $db->query("SELECT * FROM Persons WHERE firstname = :firstname",array("firstname"=>"John","id"=>"1"));
```

#### The original project offered several fetch methods such as single row, single column, single value. If you want that stuff, just write your SQL query correctly.