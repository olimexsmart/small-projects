<?php

require 'commonCode.php';

$sql = initSQLConnection();

$query = "SELECT * FROM questions ORDER BY RAND() LIMIT 60";
$result = queryWithError($sql, $query);
$questions = [];
while ($row = $result->fetch_array()) {
  $questionPacket = [];
  $questionPacket['question'] = $row['question'];
  $questionPacket['ID'] = $row['ID'];
  $questionID = $row['ID'];

  $query = "(SELECT answer, correct FROM answers 
    WHERE questID = $questionID AND correct = 1 
    ORDER BY RAND() LIMIT 1) 
    UNION 
    (SELECT answer, correct FROM answers 
    WHERE questID = $questionID AND correct = 0 
    ORDER BY RAND() LIMIT 2)";
    $randomAnswers = queryWithError($sql, $query);
    while ($a = $randomAnswers->fetch_assoc()) {
      $questionPacket['answers'][] = $a;
    }
    shuffle($questionPacket['answers']);
    $questions[] = $questionPacket;
}

header('Content-Type: application/json; charset=utf-8');
echo json_encode($questions);

