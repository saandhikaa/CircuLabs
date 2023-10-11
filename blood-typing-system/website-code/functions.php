<?php
    require 'connections.php';
    date_default_timezone_set("Asia/Jakarta");


    function querying($value){
        global $connection;
        $result = mysqli_query($connection, $value);
        $rows = [];
        while ($row = mysqli_fetch_assoc($result)){
            $rows[] = $row;
        }
        return $rows;
    }

    function credentials($id){
        global $connection;
        return mysqli_fetch_assoc(mysqli_query($connection, "SELECT * FROM credentials WHERE id = '$id'"))["key_s"];
    }

    function adding($value){
        $getid = $value["idtime"];
        $groups = strtoupper($value["groups"]);
        $rhesus = strtoupper($value["rhesus"]) === "POSITIF" ? "+" : "-";
        $tms = date("d/m/Y -- H:i:s", $getid) . " WIB";

        global $connection;
        if (mysqli_fetch_assoc(mysqli_query($connection, "SELECT id FROM blood_groups WHERE id = '$getid'"))){
            return false;
        }

        $query = "INSERT INTO blood_groups (id, ts, groups, rhesus, statuss) VALUES ('$getid', '$tms', '$groups', '$rhesus', 'unread')";
        mysqli_query($connection, $query);

        return mysqli_affected_rows($connection);
    }

    function updating($value){
        $getid = htmlspecialchars($value["id"]);
        $upname = trim(htmlspecialchars($value["nama"]));
        $upalamat = trim(htmlspecialchars($value["alamat"]));
        $uptelp = trim(htmlspecialchars($value["telp"]));

        global $connection;
        if (!empty($upname)){
            $query = "UPDATE blood_groups SET nama = '$upname', statuss = 'accepted', alamat = '$upalamat', telp = '$uptelp' WHERE id = $getid";
            mysqli_query($connection,$query);
        }

        return mysqli_affected_rows($connection);
    }

    function hiderow($id){
        global $connection;

        $query = "UPDATE blood_groups SET statuss = 'deleted' WHERE id = $id";
        mysqli_query($connection,$query);
        
        return mysqli_affected_rows($connection);
    }

    function deleterow($id){
        global $connection;

        $query = "DELETE FROM blood_groups WHERE id = $id";
        mysqli_query($connection, $query);

        return mysqli_affected_rows($connection);
    }

    function checking(){
        $url = credentials("exlink") . '/collecting.php';
        $data = array('from' => credentials("user0"));

        $options = array(
            'http' => array(
                'header' => "Content-type: application/x-www-form-urlencoded\r\n",
                'method' => "POST",
                'content' => http_build_query($data)
            )
        );

        $context = stream_context_create($options);
        $send = file_get_contents($url,false,$context);
        $datain = explode("<br>",$send);
        
        for ($s = 0; $s < count($datain)-1; $s++){
            $datains = explode(" ",$datain[$s]);
            $value["idtime"] = $datains[0];
            $value["groups"] = $datains[1];
            $value["rhesus"] = $datains[2];
            adding($value);
        }
    }

    
    function directmess($data){
        $messages = array('sender' => $data['sender'], 'message' => $data['message']);

        $options = array(
            'http' => array(
                'header' => "Content-type: application/x-www-form-urlencoded\r\n",
                'method' => "POST",
                'content' => http_build_query($messages)
            )
        );

        $context = stream_context_create($options);
        file_get_contents(credentials("api1"),false,$context);
    }


    function availableCheck($header,$table,$value){
        global $connection;
        $result = mysqli_fetch_assoc(mysqli_query($connection, "SELECT * FROM $table WHERE $header = '$value'"));
        return $result ? $result : 0 ;
    }


    function registration($data){
        global $connection;
        $getid = time();
        $nama = $data['nama'];
        $username = strtolower(stripslashes($data["username"]));
        $password = mysqli_real_escape_string($connection, $data["password"]);
        $password2 = mysqli_real_escape_string($connection, $data["password2"]);

        // cek ketersediaan username
        $result = mysqli_query($connection, "SELECT username FROM users WHERE username = '$username'");
        if (mysqli_fetch_assoc($result)){
            echo "<script>alert('Username sudah dipakai')</script>";
            return false;
        }
        
        // cek kesamaan password
        if ($password !== $password2){
            echo "<script>alert('Password tidak sama')</script>";
            return false;
        }

        // enkripsi password
        $password = password_hash($password, PASSWORD_DEFAULT);
        
        // tambahkan user baru ke database, tabel users
        $query = "INSERT INTO users (id, nama, username, password, role) VALUES ('$getid', '$nama', '$username', '$password', 'Administrator')";
        mysqli_query($connection, $query);

        $newuser = querying("SELECT * FROM users WHERE username LIKE '$username'")[0];
        
        return !empty($newuser) ? true : false;
    }
?>