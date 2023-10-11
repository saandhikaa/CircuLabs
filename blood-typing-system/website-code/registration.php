<?php
    require 'functions.php';

    if (isset($_POST["daftar"])){
        registration($_POST);
    }
    
    $administrator = querying("SELECT * FROM users");
    if (!empty($administrator)){
        header("Location: login.php");
    }
?>

<!DOCTYPE html>
<html>
<head>
    <title>Daftar</title>
    <style>
        label{ display: block; }
    </style>
</head>

<body>
    <h1>Halaman Registrasi</h1>
    <a href="index.php">goto home</a>
    <form action="" method="POST">
        <ul>
            <li>
                <label for="name">Nama: </label>
                <input type="text" name="nama" id="name" required>
                <br><br>
            </li>
            <li>
                <label for="username">Username: </label>
                <input type="text" name="username" id="username" required>
                <br><br>
            </li>

            <li>
                <label for="password">Password: </label>
                <input type="password" name="password" id="password" required>
                <br><br>
            </li>
            
            <li>
                <label for="password2">Konfirmasi password: </label>
                <input type="password" name="password2" id="password2" required>
                <br><br>
            </li>
            
            <button type="submit" name="daftar">Daftar</button>
        </ul>
    </form>  
</body>
</html>