<?php

/**
 * Initialize connection to the DB. 
 * Sets error code 502 if fails.
 */
function initSQLConnection()
{
  require 'login.php';
  // Create connection
  $sql = new mysqli($hostName, $username, $password, $dbname);
  // Check connection
  if ($sql->connect_error) {
    http_response_code(502);
    die("Connection failed: " . $sql->connect_error);
  }
  return $sql;
}

/**
 * Incapsultes query with error managing.
 * Returns the result of the query.
 */
function queryWithError($sql, $query)
{
  if (!$result = $sql->query($query)) {
    http_response_code(506);
    die("Error: " . $query . " " . $sql->error . "\n");
  }
  return $result;
}

/**
 * Verify token, incrementing the totRequest of this user
 */
function verifyToken($sql, $admin)
{
  // Parse Authorization header to get token
  $allHeaders = getallheaders();
  $map = array();
  // Create a map to handle headers in a case insensitive manner
  foreach ($allHeaders as $key => $value) {
    $map[strtolower($key)] = $key;
  }
  if (array_key_exists('authorization', $map)) {
    $token = explode(" ", $allHeaders[$map['authorization']])[1];
  } else {
    http_response_code(400);
    die("Request does not contain authorization header");
  }
  // Verify if token is present 
  $query = "SELECT isAdmin, archived FROM users WHERE token = '$token';";
  $result = queryWithError($sql, $query);
  if ($result->num_rows > 0) {
    $row = $result->fetch_assoc();
    $isAdmin = intval($row['isAdmin']);
    $archived = intval($row['archived']);
    // Increment totRequests for this user
    $query = "UPDATE users SET totRequests = totRequests + 1 WHERE token = '$token';";
    queryWithError($sql, $query);
    // Return error if archived
    if ($archived === 1) {
      http_response_code(410);
      die('Archived user');
    }
    // See if we have it is an admin too
    if ($admin && !$isAdmin) {
      http_response_code(423);
      die("Not an admin user: $token");
    }
  } else {
    http_response_code(403);
    die("Token not recognized: $token");
  }
}