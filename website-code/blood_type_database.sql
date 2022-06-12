-- phpMyAdmin SQL Dump
-- version 5.1.1
-- https://www.phpmyadmin.net/
--
-- Host: 127.0.0.1
-- Generation Time: Jun 11, 2022 at 08:14 PM
-- Server version: 10.4.22-MariaDB
-- PHP Version: 8.1.2

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `blood_type_database`
--

-- --------------------------------------------------------

--
-- Table structure for table `blood_groups`
--

CREATE TABLE `blood_groups` (
  `id` varchar(20) NOT NULL,
  `ts` varchar(30) NOT NULL,
  `groups` varchar(2) NOT NULL,
  `rhesus` varchar(2) NOT NULL,
  `statuss` varchar(10) NOT NULL,
  `nama` varchar(50) NOT NULL,
  `alamat` varchar(100) NOT NULL,
  `telp` varchar(20) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Dumping data for table `blood_groups`
--

INSERT INTO `blood_groups` (`id`, `ts`, `groups`, `rhesus`, `statuss`, `nama`, `alamat`, `telp`) VALUES
('1646992560', '11/03/2022 -- 16:56:00 WIB', 'A', '+', 'accepted', 'Mary Shelley', '', ''),
('1646992624', '11/03/2022 -- 16:57:04 WIB', 'AB', '+', 'accepted', 'Seele Vollerei', 'Moscow, Russia', '+788540321'),
('1647143632', '13/03/2022 -- 10:53:52 WIB', 'O', '-', 'accepted', 'Robert Peary', '', ''),
('1647238985', '14/03/2022 -- 13:23:05 WIB', 'B', '+', 'accepted', 'Nobel', '', ''),
('1647246649', '14/03/2022 -- 15:30:49 WIB', 'O', '+', 'accepted', 'Ana Schariac', '', ''),
('1647401820', '16/03/2022 -- 10:37:00 WIB', 'A', '+', 'accepted', 'Irene Adler', '', ''),
('1647408477', '16/03/2022 -- 12:27:57 WIB', 'A', '+', 'accepted', 'Isaac Newton', '', ''),
('1647408572', '16/03/2022 -- 12:29:32 WIB', 'A', '-', 'accepted', 'Nikola Tesla', '', ''),
('1647410861', '16/03/2022 -- 13:07:41 WIB', 'AB', '+', 'accepted', 'Elizabeth Bathory', 'Inggris', '+4405561971'),
('1647412347', '16/03/2022 -- 13:32:27 WIB', 'O', '+', 'accepted', 'Planck', 'Inggris', '+44076121290'),
('1647429154', '16/03/2022 -- 18:12:34 WIB', 'B', '+', 'accepted', 'Beethoven', 'Jerman', '+491234567890'),
('1647429279', '16/03/2022 -- 18:14:39 WIB', 'A', '-', 'accepted', 'Edwin Hubble', 'Amerika', ''),
('1647429293', '16/03/2022 -- 18:14:53 WIB', 'O', '+', 'accepted', 'Dirac', '', ''),
('1647429306', '16/03/2022 -- 18:15:06 WIB', 'AB', '+', 'accepted', 'Einstein', 'Jerman', ''),
('1647429319', '16/03/2022 -- 18:15:19 WIB', 'A', '-', 'accepted', 'Ekaterina', '', ''),
('1647429482', '16/03/2022 -- 18:18:02 WIB', 'A', '+', 'accepted', 'Gustav Klimt', 'Austria', ''),
('1647429631', '16/03/2022 -- 18:20:31 WIB', 'A', '-', 'accepted', 'Lier Scarlet', '', '+5987654321'),
('1647429759', '16/03/2022 -- 18:22:39 WIB', 'A', '-', 'accepted', 'Mendeleev', '', ''),
('1647442517', '16/03/2022 -- 21:55:17 WIB', 'AB', '+', 'accepted', 'Charlemagne', 'Eropa', ''),
('1647443148', '16/03/2022 -- 22:05:48 WIB', 'A', '+', 'unread', '', '', ''),
('1647445740', '16/03/2022 -- 22:49:00 WIB', 'O', '+', 'unread', '', '', ''),
('1654697024', '08/06/2022 -- 21:03:44 WIB', 'O', '-', 'unread', '', '', '');

-- --------------------------------------------------------

--
-- Table structure for table `blood_groups_tmp`
--

CREATE TABLE `blood_groups_tmp` (
  `id` varchar(20) NOT NULL,
  `ts` varchar(30) NOT NULL,
  `groups` varchar(2) NOT NULL,
  `rhesus` varchar(10) NOT NULL,
  `statuss` varchar(10) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- Table structure for table `credentials`
--

CREATE TABLE `credentials` (
  `id` varchar(20) NOT NULL,
  `key_s` varchar(100) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Dumping data for table `credentials`
--

INSERT INTO `credentials` (`id`, `key_s`) VALUES
('exlink', 'optional'),
('user0', 'default'),
('user1', 'arduino');

-- --------------------------------------------------------

--
-- Table structure for table `users`
--

CREATE TABLE `users` (
  `id` varchar(20) NOT NULL,
  `nama` varchar(25) NOT NULL,
  `username` varchar(20) NOT NULL,
  `password` varchar(255) NOT NULL,
  `role` varchar(20) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- Indexes for dumped tables
--

--
-- Indexes for table `blood_groups`
--
ALTER TABLE `blood_groups`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `blood_groups_tmp`
--
ALTER TABLE `blood_groups_tmp`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `credentials`
--
ALTER TABLE `credentials`
  ADD PRIMARY KEY (`id`);

--
-- Indexes for table `users`
--
ALTER TABLE `users`
  ADD PRIMARY KEY (`username`);
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
