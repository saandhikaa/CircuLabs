<?php
    require 'functions.php';
    session_start();

    // get new data
    // checking();

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

    // get user info
    if (!empty($_SESSION['login'])){
        $getid = $_SESSION['login'];
        $users = querying("SELECT * FROM users WHERE id LIKE '$getid'")[0];
        $newdata = querying("SELECT * FROM blood_groups WHERE statuss LIKE 'unread'");
    }
    
    // get detail info
    if (!empty($_GET["getdetail"])){
        $getid = $_GET["getdetail"];
        $detaileddata = querying("SELECT * FROM blood_groups WHERE id LIKE '$getid'")[0];
    }
    
    $TLine = 10;
    $CPage = !empty($_GET["page"]) ? $_GET["page"] : 1 ;
    $SRows = $TLine * $CPage - $TLine;

    if (!empty($_GET['show'])){
        if ($_GET['show'] != "all"){
            $shows = $_GET['show'];
            $TRows = count(querying("SELECT * FROM blood_groups WHERE statuss LIKE 'accepted' AND groups LIKE '$shows'"));
            $accepteddata = querying("SELECT * FROM blood_groups WHERE statuss LIKE 'accepted' AND groups LIKE '$shows' ORDER BY id DESC LIMIT $SRows, $TLine");
        }else{
            $TRows = count(querying("SELECT * FROM blood_groups WHERE statuss LIKE 'accepted'"));
            $accepteddata = querying("SELECT * FROM blood_groups WHERE statuss LIKE 'accepted' ORDER BY id DESC LIMIT $SRows, $TLine");
        }
    }else{
        header("location: index.php?show=all");
    }

    $TPage = ceil($TRows/$TLine);
?>

<!DOCTYPE html>
<html lang="id">

<head>
    <title>Website</title>
    <link rel="stylesheet" href="styles.css">
    
    <link rel="preconnect" href="https://fonts.googleapis.com">
    <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
    <link href="https://fonts.googleapis.com/css2?family=Montserrat:wght@500&display=swap" rel="stylesheet">
    <link href="https://fonts.googleapis.com/css2?family=Signika:wght@600&display=swap" rel="stylesheet">
</head>

<body>
    <div class="container">

        <div class="header">
            <h1>DATA GOLONGAN DARAH</h1>
            <?= !empty($_SESSION['login']) ? $_SESSION['login'] : "" ?>
        </div>


        <div class="content">
            <?php if (!empty($_SESSION['login']) && !empty($newdata)): ?>
                <div class="card">
                    <h1>Tabel Data Masuk</h1>
                    <div class="mytable">
                        <table>
                            <thead>
                                <tr>
                                <th>No.</th>
                                <th>Waktu Ambil</th>
                                <th>Gol. Darah</th>
                                <th class="opt">Pilihan</th>
                                </tr>
                            </thead>
                            <tbody>
                                <tr class="space"><td></td><td></td><td></td><td></td></tr>
                                <?php $num = 1 ?>
                                <?php foreach($newdata as $row): ?>
                                    <tr>
                                    <td><?= $num; $num++ ?></td>
                                    <td><?= date("d/m/Y - H:i", $row["id"]) . " WIB" ?></td>
                                    <td><?= $row["groups"] . $row["rhesus"] ?></td>
                                    <td class="opt">
                                        <a href="getting.php?get=simpan&id=<?= $row["id"] ?>"><input class="simpan" type="submit" value="Simpan"></a>
                                        <a href="getting.php?get=hapus&id=<?= $row["id"] ?>" onclick="return confirm('Delete data?')"><input class="hapus" type="submit" value="Hapus"></a>
                                    </td>
                                    </tr>
                                <?php endforeach ?>
                            </tbody>
                        </table>
                    </div>
                </div><br>
            <?php endif ?>

            <div class="card">
                <h1>Tabel Data Golongan Darah</h1>
                <div class="mytable">
                <table>
                    <thead>
                        <tr>
                        <th>No.</th>
                        <th>Tanggal</th>
                        <th>Nama</th>
                        <th>Gol. Darah</th>
                        <th class="opt"></th>
                        </tr>
                    </thead>
                    <tbody>
                        <tr class="space"><td></td><td></td><td></td><td></td><td></td></tr>
                        <?php $num = 1 ?>
                        <?php foreach($accepteddata as $row): ?>
                            <?php if (!empty($detaileddata)): ?>
                                <?php if ($row["id"] == $detaileddata["id"]): ?>
                                    <tr class="selected">
                                <?php else: ?>
                                    <tr>
                                <?php endif ?>
                            <?php else: ?>
                                <tr>
                            <?php endif ?>

                            <td><?= $num; $num++ ?></td>
                            <td><?= date("d/m/Y", $row["id"]) ?></td>
                            <td><?= $row["nama"] ?></td>
                            <td><?= $row["groups"] . $row["rhesus"] ?></td>
                            <td class="opt">
                                <a href="?getdetail=<?= $row["id"] . "&page=" . $CPage . "&show=" . $_GET['show'] ?>"><input style="width: fit-content;" class="detail" type="submit" value=">"></a>
                            </td>
                            </tr>
                        <?php endforeach ?>
                    </tbody>
                </table>
                </div>

                <!-- pagination -->
                <?php if ($TPage > 1): ?>
                    <div class="pagging">
                        <?php for ($s = 1; $s <= $TPage; $s++): ?>
                            <a href="?page=<?= $s . "&show=" . $_GET['show'] ?>">
                                <?php if ($s == $CPage): ?>
                                    <input class="spage" type="submit" value="<?= $s ?>">
                                <?php else: ?>
                                    <input class="page" type="submit" value="<?= $s ?>">
                                <?php endif ?>
                            </a>
                        <?php endfor ?>
                    </div>
                <?php endif ?>
            </div>
        </div>



        <div class="sidebar">
            <div class="users">
                <h2>Navigation</h2>
                <?php if (!empty($_SESSION['login'])): ?>
                    <div style="margin-top: 20px ;"><label><?= $users['nama'] ?></label></div>
                    <div style="margin-top: 5px ;"><label style="font-weight: normal ;"><?= $users['role'] ?></label></div>
                <?php endif ?>

                <div style="margin-top: 25px; margin-bottom: 5px ;"><label>Data Table</label></div>
                <div class="menu-show">
                    <?php $type = ["all","A","B","AB","O"] ?>
                    <?php for ($s = 0; $s < 5; $s++): ?>
                        <a href="?show=<?= $type[$s] ?>"><input style="font-family: montserrat;" type="submit" value="Show <?php if($s > 0){echo "type ";} echo $type[$s] . " "; if($type[$s] == $_GET['show']){echo " ◄";} ?>"></a><br>
                    <?php endfor ?>
                </div><br>

                <?php if (!empty($_SESSION['login'])): ?>
                    <div class="link"><a href="manage.php"><input type="submit" value="MANAGE DATA"></a></div>
                    <div class="link"><a href="getting.php?get=logoutme"><input type="submit" value="LOGOUT"></a></div><br>
                <?php else: ?>
                    <div class="link"><a href="login.php"><input type="submit" value="LOGIN"></a></div><br>
                <?php endif ?>
            </div>

            <?php if (!empty($detaileddata)): ?>
                <div class="details">
                    <div class="exit"><a href="?page=<?= $CPage ?>"><input type="submit" value="X"></a></div>
                    <h2>Detail</h2>
                    <div class="detailbox"><br>
                        <label>Nama</label>
                        <p><?= $detaileddata["nama"] ?></p>
                    </div>
                    <div class="detailbox">
                        <label>Golongan Darah</label>
                        <p><?= $detaileddata["groups"] . $detaileddata["rhesus"] ?></p>
                    </div>
                    <div class="detailbox">
                        <label>Tanggal Ambil</label>
                        <p><?= date("d/m/Y", $detaileddata["id"]) ?></p>
                    </div>

                    <?php if (!empty($_SESSION['login'])): ?>
                        <?php if (!empty($detaileddata["alamat"])): ?>
                            <div class="detailbox">
                                <label>Alamat</label>
                                <p><?= $detaileddata["alamat"] ?></p>        
                            </div>
                        <?php endif ?>
                        <?php if (!empty($detaileddata["telp"])): ?>
                            <div class="detailbox">
                                <label>No. Telepon</label>
                                <p><?= $detaileddata["telp"] ?></p>        
                            </div>
                        <?php endif ?>
                        <div class="link"><a href="getting.php?get=edit&id=<?= $detaileddata["id"] ?>"><input class="edit" type="submit" value="Edit"></a></div>
                        <div class="link"><a href="getting.php?get=hapus&id=<?= $detaileddata["id"] ?>" onclick="return confirm('Delete data?')"><input class="hapus" type="submit" value="Hapus"></a></div>
                    <?php endif ?><br>
                </div>
            <?php endif ?><br><br>
        </div>

        <div class="footer">
            <div class="endline"> </div>
            <div class="menu">
                <a href="about.php"><input type="submit" value="About"></a>
                <a href="mailto:farras.live@outlook.com?subject=Github-Code,%20Database%20Golongan%20Darah"><input type="submit" value="Email me"></a>
            </div>
            <div class="copyright">
                <p>Copyright © 2022 - <?= $_SERVER['SERVER_NAME'] ?> - All Rights Reserved</p>
            </div>
        </div>

    </div>
</body>
</html>