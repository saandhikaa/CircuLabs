<?php
    require 'connections.php';
    
    function credentials($id){
        global $connection;
        return mysqli_fetch_assoc(mysqli_query($connection, "SELECT * FROM credentials WHERE id = '$id'"))["key_s"];
    }

    $json = file_get_contents('php://input');
    $object = json_decode($json);
    
    if (isset($object -> groups) > 0 && isset($object -> rhesus) > 0){
        $value["groups"] = $object -> groups;
        $value["rhesus"] = $object -> rhesus;
        $value["idtime"] = time();
        
        echo adding($value)  ? "inserted (POST JSON)  " : "failed  ";
    }
    
    if (!empty($_POST["from"])){
        // from api
        if ($_POST["from"] == credentials("user1") && !empty($_POST["groups"]) && !empty($_POST["rhesus"]) && !empty($_POST["direct"])){
            $getid = isset($_POST["idtime"]) ? strtotime($_POST["idtime"]) : time() ;
            $groups = strtoupper($_POST["groups"]);
            date_default_timezone_set("Asia/Jakarta");
            $tms = date("d/m/Y -- H:i:s", ($getid)) . " WIB";
            
            if (strtoupper($_POST["direct"]) == "YES"){
                $table = "blood_groups";
                $rhesus = strtoupper($_POST["rhesus"]) == "POSITIF" ? "+" : "-" ;
            }else{
                $table = "blood_groups_tmp";
                $rhesus = strtoupper($_POST["rhesus"]);
            }

            mysqli_query($connection, "INSERT INTO " . $table . " (id, ts, groups, rhesus, statuss) VALUES ('$getid', '$tms', '$groups', '$rhesus', 'unread')");
            
            echo $getid . " " . $groups . " " . $_POST['rhesus'];
            echo mysqli_affected_rows($connection) == 1 ? " -- inserted" : " -- failed";
        }

        // from other website
        if ($_POST["from"] == credentials("user0")){
            $result = mysqli_query($connection, "SELECT * FROM blood_groups_tmp WHERE statuss LIKE 'unread'");
            $rows = [];
            while ($row = mysqli_fetch_assoc($result)){
                $rows[] = $row;
            }

            foreach ($rows as $row){
                $id = $row["id"];
                echo $row["id"] . " " . $row["groups"] . " " . $row["rhesus"] . "<br>";
                mysqli_query($connection,"UPDATE blood_groups_tmp SET statuss = 'sent' WHERE id = $id");
            }
        }

    }

?>