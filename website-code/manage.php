<?php
    require 'functions.php';
    session_start();

    empty($_SESSION["login"]) ? header("Location: login.php") : 0 ;

    $alldata = querying("SELECT * FROM blood_groups");

    // $tables = querying("SHOW TABLES");
    // for ($s = 0; $s < count($tables); $s++) echo $tables[$s]["Tables_in_" . $dbname] . "<br>";
?>

<!DOCTYPE html>
<html>
<head>
    <title>Manage</title>

    <link rel="preconnect" href="https://fonts.googleapis.com">
    <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
    <link href="https://fonts.googleapis.com/css2?family=Montserrat:wght@500&display=swap" rel="stylesheet">
    <link href="https://fonts.googleapis.com/css2?family=Signika:wght@500&display=swap" rel="stylesheet">
    <link href="https://fonts.googleapis.com/css2?family=Fira+Code:wght@300&display=swap" rel="stylesheet">

    <style>
        body{
            width: max-content;
            padding-bottom: 50px;
        }
        h1{
            font-family: Signika;
            font-size: 40px;
        }
        table th{
            font-family: Signika;
            font-weight: normal;
            color: white;
            padding: 10px 10px;
            text-align: center;
        }
        table td{
            padding: 5px 10px;
            font-family: Montserrat;
            font-size: 14px;
        }
        .endline td{
            border-top: 2px solid #067D68;;
        }
        
    </style>
</head>

<body>
    <h1>ALL DATA ON DATABASE</h1>
    <a href="index.php">Back to home</a><br><br>
    <table>
        <thead>
            <tr style="background-color: #067D68;">
                <th>NO.</th>
                <th>TIME STAMP</th>
                <th>NAME</th>
                <th>BLOOD TYPE</th>
                <th>ADDRESS</th>
                <th>TELP.</th>
                <th>STATUS</th>
                <th>ACTION</th>
            </tr>
        </thead>
        <tbody>
            <?php for ($s = 0; $s < count($alldata); $s++): ?>
                <tr style="<?php if ($s % 2){ echo "background-color: rgb(212, 212, 212);"; }else{echo "background-color: rgb(241, 241, 241);"; } ?>">
                    <td style="font-family: Fira Code; text-align: right;"><?= $s+1 ?></td>
                    <td style="font-family: Fira Code;"><?= $alldata[$s]['ts'] ?></td>
                    <td><?= $alldata[$s]['nama'] ?></td>
                    <td><?= $alldata[$s]['groups'] . $alldata[$s]['rhesus'] ?></td>
                    <td style="max-width: 340px;"><?= $alldata[$s]['alamat'] ?></td>
                    <td><?= $alldata[$s]['telp'] ?></td>
                    <td style="text-align:center; <?php if ($alldata[$s]['statuss'] == "deleted"){ echo "color: red"; }elseif($alldata[$s]['statuss'] == "accepted"){echo "color: green;"; } ?>"><?= $alldata[$s]['statuss'] ?></td>
                    <td>
                        <a href="getting.php?id=<?= $alldata[$s]['id'] ?>&admin=delete" onclick="return confirm('remove <?= $alldata[$s]['nama'] ?>?')">remove</a> / 
                        <a href="getting.php?id=<?= $alldata[$s]['id'] ?>&admin=unread" onclick="return confirm('change status to deleted?')">hide</a>
                    </td>
                </tr>
            <?php endfor ?>
            <tr class="endline"><td></td><td></td><td></td><td></td><td></td><td></td><td></td><td></td></tr>
        </tbody>
    </table>
</body>
</html>