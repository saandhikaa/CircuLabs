<?php
    require 'functions.php';
    
    session_start();

    $administrator = querying("SELECT * FROM users");
    if (empty($administrator)){
        header("Location: registration.php");
    }

    // get session user id if cookie was saved
    if (isset($_COOKIE['id']) && isset($_COOKIE['key']) && empty($_SESSION['login'])){
        $id = $_COOKIE['id'];
        $key = $_COOKIE['key'];

        $result = mysqli_query($connection, "SELECT username FROM users WHERE id = $id");
        $cell = mysqli_fetch_assoc($result);

        if ($key === hash('sha256', $cell['username'])){
            $_SESSION['login'] = $id;
        }
    }

    !empty($_SESSION['login']) ? header("Location: index.php") : 0;

    if (isset($_POST["login"])){
        $username = strtolower(stripslashes($_POST["username"]));
        $password = $_POST["password"];
        $result = mysqli_query($connection, "SELECT * FROM users WHERE username = '$username'");
        if (mysqli_num_rows($result) === 1){
            $users = mysqli_fetch_assoc($result);
        
            if (password_verify($password, $users["password"])){
                $getid = $users["id"];
                $_SESSION['login'] = $getid;
                
                if (!empty($_POST["remember"])){
                    $duration = 30; //days
                    setcookie('id', $users['id'], $duration*24*60*60 + time());
                    setcookie('key', hash('sha256', $users['username']), $duration*24*60*60 + time());
                }

                if (!empty($_POST['idtime']) && !empty($_POST['groups']) && !empty($_POST['rhesus'])){
                    $value = ["idtime" => $_POST['idtime'], "groups" => $_POST['groups'], "rhesus" => $_POST['rhesus']];
                    adding($value);
                }
                
                header("location: index.php");
                exit;
            }
        }
        $error = true;
        
    }
?>

<!DOCTYPE html>
<html>
<head>
    <title>Login</title>
    <link rel="stylesheet" href="stylesext.css">
    
    <link rel="preconnect" href="https://fonts.googleapis.com">
    <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
    <link href="https://fonts.googleapis.com/css2?family=Montserrat:wght@500&display=swap" rel="stylesheet">

    <style>
        body{
            margin: 0;
            padding: 0;
            background: linear-gradient(120deg, #50D5B7, #067D68);
            height: 100vh;
        }

    </style>
</head>

<body>
    <div class="login">
        <h1>LOGIN</h1>
        <form action="" method="POST">
            <?php if (!empty($_POST['data'])): ?>
                <input type="hidden" name="idtime" value="<?= $_POST['idtime'] ?>">
                <input type="hidden" name="groups" value="<?= $_POST['groups'] ?>">
                <input type="hidden" name="rhesus" value="<?= $_POST['rhesus'] ?>">
            <?php endif ?>

            <div class="textbox">
                <input type="text" name="username" autofocus required>
                <span></span>
                <label>Username</label>
            </div>

            <div class="textbox">
                <input type="password" name="password" required>
                <span></span>
                <label>Password</label>
            </div>

            <div class="error">
                <label><?= isset($error) ? "*Incorrect username or password" : "" ?></label>
            </div>

            <div class="checkbox">
                <input type="checkbox" name="remember" id="remember">
                <label style="font-size: 14px;" for="remember">Remember me</label>    
            </div>
            
            <input type="submit" name="login" value="Login">
        </form>

    </div>

</body>

</html>