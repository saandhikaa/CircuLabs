<?php
    require 'functions.php';
    session_start();
    
    empty($_SESSION["login"]) ? header("Location: login.php") : 0 ;

    $bulan = ["januari", "februari", "maret", "april", "mei", "juni", "juli", "agustus", "september", "oktober", "november", "desember"];
    $duplicate = 0;

    if ($_GET["get"] == "logoutme"){
        session_unset();
        session_destroy();

        setcookie('id', '', time() - 3600);
        setcookie('key', '', time() - 3600);

        header("Location: index.php");
        exit;
    }

    if ($_GET["get"] == "hapus"){
        hiderow($_GET["id"]);
        header("Location: index.php");
        exit;
    }

    if (!empty($_GET['admin'])){
        if ($_GET['admin'] == "delete"){
            deleterow($_GET['id']);
        }
        if ($_GET['admin'] == "unread"){
            hiderow($_GET['id']);
        }
        header("Location: manage.php");
        exit;
    }

    if (isset($_POST["simpan"])){
        if ($_POST["simpan"] == "Simpan"){
            $tmpnama = $_POST['nama'];
            $duplicate = availableCheck("nama", "blood_groups", "$tmpnama");
            
            if ($duplicate){
                if ($duplicate['id'] == $_POST['id'] || $duplicate['groups'] != $_POST['groups'] || $duplicate['rhesus'] != $_POST['rhesus']){
                    updating($_POST);
                    header("Location: index.php");
                }
            }else{
                updating($_POST);
                header("Location: index.php");
            }

        }elseif ($_POST["simpan"] == "Batal"){
            header("Location: index.php");

        }elseif ($_POST["simpan"] == "Hapus"){
            hiderow($_POST['id']);
            header("Location: index.php");
        }
    }



    $getid = $_GET["id"];
    $row = querying("SELECT * FROM blood_groups WHERE id LIKE '$getid'")[0];

?>

<!DOCTYPE html>
<html lang="id">
<head>
    <title>Update Data</title>
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
    <div class="getsimpan">
        <?php if ($duplicate): ?>
            <h1>Data sama ditemukan</h1><br>
            <p>
                Data yang Anda masukkan serupa dengan data yang diambil pada tanggal 
                <span class="highlight"><?php $tmp = $bulan[date("n", $duplicate['id']) - 1]; echo date("d ", $duplicate['id']) . $tmp . date(" Y", $duplicate['id']) ?></span> 
                pukul 
                <span class="highlight"><?= date("H : i", $duplicate['id']) . " WIB"?></span>, 
                yaitu milik
                <span class="highlight"><?= $duplicate['nama'] ?></span>
                (<span class="highlight"><?= $duplicate['groups'] . $duplicate['rhesus'] ?></span>)
                <?php if (!empty($duplicate['alamat'])): ?>
                    <span>yang beralamat di</span> 
                    <span class="highlight"><?= $duplicate['alamat'] ?></span>
                <?php endif ?>
                <?php if (!empty($duplicate['telp'])): ?>
                    <span>dengan nomor telepon </span> 
                    <span class="highlight"><?= $duplicate['telp'] ?></span>
                <?php endif ?>.
            </p>
            <p>Apa yang akan Anda lakukan pada data baru?</p>
            
            <form action="" method="POST">
                <div class="link">
                    <input type="hidden" name="id" value="<?= $_POST['id'] ?>">
                    <input type="hidden" name="nama" value="<?= $_POST['nama'] ?>">
                    <input type="hidden" name="alamat" value="<?= $_POST['alamat'] ?>">
                    <input type="hidden" name="telp" value="<?= $_POST['telp'] ?>">
                    
                    <input type="submit" name="simpan" value="Simpan">
                    <input type="submit" name="simpan" value="Hapus">
                    <input type="submit" name="simpan" value="Batal">
                </div>
            </form>

        <?php else: ?>
            <h1>Simpan Data</h1>
            <form action="" method="POST">
                <input type="hidden" name="id" value="<?= $row["id"] ?>">
                <input type="hidden" name="groups" value="<?= $row["groups"]?>">
                <input type="hidden" name="rhesus" value="<?= $row["rhesus"]?>">
                <div class="getbox">
                    <label>Waktu Pengambilan</label>
                    <span></span>
                    <input type="text" value="<?= date("d/m/Y - H:i", $row["id"]) . " WIB" ?>" disabled>
                </div>
                <div class="getbox">
                    <label>Gol. Darah</label>
                    <span></span>
                    <input type="text" value="<?= $row["groups"] . $row["rhesus"] ?>" disabled>
                </div>
                <div class="getbox">
                    <label>Nama</label>
                    <span></span>
                    <?php if (!empty($row["nama"])): ?>
                        <input type="text" name="nama" value="<?= $row["nama"] ?>">
                    <?php else: ?>
                        <input type="text" name="nama" autofocus>
                    <?php endif ?>
                </div>
                <div class="getbox">
                    <label>Alamat</label>
                    <span></span>
                    <?php if (!empty($row["alamat"])): ?>
                        <input type="text" name="alamat" value="<?= $row["alamat"] ?>">
                    <?php else: ?>
                        <input type="text" name="alamat" autofocus>
                    <?php endif ?>
                </div>
                <div class="getbox">
                    <label>No. Telepon</label>
                    <span></span>
                    <?php if (!empty($row["telp"])): ?>
                        <input type="text" name="telp" value="<?= $row["telp"] ?>">
                    <?php else: ?>
                        <input type="text" name="telp" autofocus>
                    <?php endif ?>
                </div>
                <br>
                <div class="link">
                    <input type="submit" name="simpan" value="Simpan">
                    <input type="submit" name="simpan" value="Batal">
                </div>
            </form>
        <?php endif ?>
    </div>

</body>
</html>