<?php
    require 'connections.php';
    session_start();

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

    // check for duplicate data
    if (!empty($_GET["groups"]) && !empty($_GET["rhesus"] && !empty($_GET["idtime"]))){
        $groups = $_GET["groups"];
        $rhesus = $_GET["rhesus"];
        $idtime = strtotime($_GET["idtime"]);

        $result = mysqli_query($connection, "SELECT id FROM blood_groups WHERE id = '$idtime'");
        if (mysqli_fetch_assoc($result)){
            header("Location: index.php");
        }
    }

?>

<!DOCTYPE html>
<html>
<head>
    <title>Verify</title>
    
</head>
<body>
    <?php if (!empty($_SESSION['login'])): ?>
        <?php
            $value = ["idtime" => $idtime, "groups" => $groups, "rhesus" => $rhesus];
            require_once 'functions.php';
            adding($value);
            header("Location: index.php");
        ?>
        
    <?php else: ?>
        <form hidden action="login.php" method="POST">
            <input type="text" name="idtime" value="<?= $idtime ?>">
            <input type="text" name="groups" value="<?= $groups ?>">
            <input type="text" name="rhesus" value="<?= $rhesus ?>">
            <input type="submit" name="data" value="yes" id="login">
        </form>        
        <script type="text/javascript"> document.getElementById("login").click(); </script>

    <?php endif ?>

</body>
</html>