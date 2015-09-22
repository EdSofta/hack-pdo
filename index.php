<?hh
	require("DB.hh");

	// Creates the instance
	$db = new Schlunix\Pdo\DB();
	// Location of the config file
	$db->setConfigFile("settings.ini.php");
	// DB to connect to (must be in the config file)
	$db->Connect("db1");


	// 2 ways to bind parameters :

	// 1. setBindParameters
	$db->setBindParameters(Vector{Vector{"firstname", "John"},
																Vector{"age", "19"}});

	// 2. Or just give the parameters to the method
	$db->query("SELECT * FROM Persons WHERE firstname = :firstname AND age = :age",
							Vector{Vector{"firstname", "John"}, Vector{"age", "19"}});

	// 2a. You can also include the PDO Parameter type as the third param
	$db->query("SELECT * FROM Persons WHERE firstname = :firstname AND age = :age",
							Vector{Vector{"firstname", "John", "PDO::PARAM_STR"},
										 Vector{"age", "19", "PDO::PARAM_INT"}});


	//  Fetching data
	$person 	 =     $db->query("SELECT * FROM Persons");

	// If you want another fetchmode just give it as parameter
	$persons_num =     $db->query("SELECT * FROM Persons", null, PDO::FETCH_NUM);

	// Column, numeric index
	$ages  		 =     $db->column("SELECT age FROM Persons");

	// The following statemens will return the affected rows

	// Update statement
	$update		=  $db->query("UPDATE Persons SET firstname = :f WHERE Id = :id",
													 array("f"=>"Johny","id"=>"1"));

	function d($v,$t)
	{
		echo '<pre>';
		echo '<h1>' . $t. '</h1>';
		var_dump($v);
		echo '</pre>';
	}
	d($person, "All persons");
	d($firstname, "Fetch Single value, The firstname");
	d($id_age, "Single Row, Id and Age");
	d($ages, "Fetch Column, Numeric Index");
