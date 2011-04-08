<?php
// $Id$

/**
 * MediaMosa is a Full Featured, Webservice Oriented Media Management and
 * Distribution platform (http://www.vpcore.nl)
 *
 * Copyright (C) 2009 SURFnet BV (http://www.surfnet.nl) and Kennisnet
 * (http://www.kennisnet.nl)
 *
 * MediaMosa is based on the open source Drupal platform and
 * was originally developed by Madcap BV (http://www.madcap.nl)
 *
 * video_mediamosa is a transcoder for the video module and was
 * originally developed by Synetic BV (http://www.synetic.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, you can find it at:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 */

/**
 * @file
 * This module contains the login procedure, cookie handling and basic GET
 * and POST functions to communicate with MediaMosa.
 */

/**
 * mediamosa_client is a class which conains all functions for communication with MediaMosa
 */
class mediamosa_client {
  protected $mediamosaserver_uri;
  protected $cookie = FALSE;
  protected $log = array();
  protected $rest;
  protected $user_id;
  protected $group_id;
  protected $username;
  protected $password;
  protected $is_app_admin;

  public function __construct($user_id = FALSE, $group_id = FALSE, $is_app_admin = NULL) {
    $this->rest = variable_get('video_mediamosa_use_rest_interface', TRUE);
    $this->set_user($user_id, $group_id, $is_app_admin);
    if ($this->rest) {
      $this->cookie = variable_get('video_mediamosa_cookie', NULL);
      $this->set_mediamosa_server_uri(variable_get('video_mediamosa_uri', NULL));
      $this->set_auth_credentials(variable_get('video_mediamosa_username', NULL),
                                variable_get('video_mediamosa_password', NULL));
    }
  }
  
  public function set_mediamosa_server_uri($mediamosaserver_uri) {
    $this->mediamosaserver_uri = $mediamosaserver_uri;
  }
  
  public function set_auth_credentials($username, $password) {
    $this->username = $username;
    $this->password = $password;
  }
  
  public function set_user($user_id = FALSE, $group_id = FALSE, $is_app_admin = NULL) {
    global $user;
    if (!isset($user->profile_group_id) && function_exists('profile_load_profile')) {
      profile_load_profile($user);
    }
    $username = (isset($user->name)) ? $user->name : variable_get('anonymous', 'Anonymous');
    $this->user_id = ($user_id !== FALSE) ? $user_id : $username;
    $this->group_id = ($group_id !== FALSE) ? $group_id : $user->profile->group_id;
    $this->is_app_admin = ($is_app_admin !== NULL) ? $is_app_admin : user_access('wle administrator');
    $this->masquerade_owner = variable_get('video_mediamosa_masquerade_owner', FALSE);
  }
  
  public function get_user() {
    return Array($this->group_id, $this->user_id);
  }

  protected function add_user_info($uri, &$data = NULL) {
    $uri .= (strpos($uri, '?') !== FALSE) ? '&' : '?';
    $uri .= 'user_id='. $this->user_id;
    if ($this->group_id != '') {
      $uri .= '&group_id='. $this->group_id;
    }
    if ($this->masquerade_owner || $this->is_app_admin) {
      $uri .= '&is_app_admin=TRUE';
      if ($data) {
        if (is_array($data)) {
          $data['is_app_admin'] = 'TRUE';
        }
        else {
          $data .= '&is_app_admin=TRUE';
        }
      }
    }
    return $uri;
  }

  protected function _login() {
    // step 1: request the challenge
    $response = $this->_request('POST', '/login', array('dbus' => 'AUTH DBUS_COOKIE_SHA1 '. $this->username));

    // parse the response
    list($this->cookie) = explode(";", $response->headers['Set-Cookie']);
    variable_set('video_mediamosa_cookie', $this->cookie);
    preg_match('@<dbus>DATA vpx 0 (.*)</dbus>@', $response->data, $matches);
    $challenge = $matches[1];

    // step 2: send credentials
    $challenge_response = sha1(sprintf('%s:%s:%s', $challenge, '', $this->password));
    $response = $this->_request('POST', '/login', array('dbus' => sprintf('DATA %s %s', '', $challenge_response)));

    // parse the response
    preg_match('@<dbus>(.*)</dbus>@', $response->data, $matches);
    $result = $matches[1];

    // return TRUE or FALSE
    return (drupal_substr($result, 0, 2) === 'OK');
  }
  
  protected function _check_login($xml) {
    preg_match('@<request_result_id>(.*)</request_result_id>@', $xml, $matches);
    return (empty($matches) || $matches[1] != '1601'); // 1601 == not logged in
  }

  public function request($method, $uri, $data = NULL, $process_result = TRUE, $app_id = FALSE) {
    if ($this->rest) {
      // Add user info to the uri
      $uri = $this->add_user_info($uri);

      // first try to communicate with MediaMosa without logging in
      if (($response = $this->_request($method, $uri, $data)) === FALSE) { // request failed
        
        return FALSE;
      }
      $xml = $response->data;

      // parse the request_result_id of the response
      if (substr($xml, 0, 1) == '<' && $this->_check_login($xml) === FALSE) {
        $this->_login();
        $response = $this->_request($method, $uri, $data); // perform the same request again
        $xml = $response->data;
      }
      // parse the request_result_id of the response
      if (substr($xml, 0, 1) == '<' && $this->_check_login($xml) === FALSE) {
        return FALSE;
      }
    }
    else { // The non-REST, direct connection to MediaMosa
      // Add user info to the uri
      $uri = $this->add_user_info($uri, $data);

      // Gather all GET and POST parameters
      $a_uri = explode('?', $uri);
      $uri = drupal_substr($a_uri[0], 1);
      (isset($a_uri[1])) ? $get = explode('&', $a_uri[1]) : $get = array();

      $args = array();
      foreach ($get as $get_item) {
        $a_get_item = explode('=', $get_item);
        if (strpos($a_get_item[0], '[]') === drupal_strlen($a_get_item[0]) - 2) { // detect array notation
          $args['get'][drupal_substr($a_get_item[0], 0, -2)][] = rawurldecode($a_get_item[1]);
        }
        else {
          $args['get'][$a_get_item[0]] = rawurldecode($a_get_item[1]);
        }
      }
      if ($data) {
        if (is_array($data)) {
          $args['post'] = $data;
        }
        else {
          $post = explode('&', $data);

          foreach ($post as $post_item) {
            $a_post_item = explode('=', $post_item);
            if (strpos($a_post_item[0], '[]') === drupal_strlen($a_post_item[0]) - 2) { // detect array notation
              $args['post'][substr($a_post_item[0], 0, -2)][] = rawurldecode($a_post_item[1]);
            }
            else {
              $args['post'][$a_post_item[0]] = rawurldecode($a_post_item[1]);
            }
          }
        }
      }

      // Get all app_id's available for this user
      if ($app_id === FALSE || !$this->masquerade_owner) {

        global $user;

        $resource = db_query("SELECT id, owner FROM {client_applications} WHERE owner IN ('', '". implode("', '", $user->roles) ."')");
        $app_ids = array();

        // Filter out test apps when we are not super user (uid == 1)
        $app_ids_to_skip = ($user->uid == 1 ? array() : mediamosafield_shared_is_simpletest_app_get());

        while ($result = db_fetch_array($resource)) {
          if (!in_array($result['id'], $app_ids_to_skip)) {
            $app_ids[] = $result['id'];
          }
        }

        $args['get']['app_id'] = $app_ids;
      }
      else {
        $args['get']['app_id'] = $app_id;
      }

      // Get the MadREST definition
      $madrest_definition = madrest_get_uri_definition($uri, explode('/', $uri), $args['uri'], $method);

      // Perform the request
      $start = microtime(TRUE);
      $output = call_user_func($madrest_definition['function_call'], $args);
      $xml = $output->create_output($start);

      // Log the request
      if (variable_get('video_mediamosa_debug', FALSE)) {
        $this->log[] = array(
          'uri' => $madrest_definition['function_call'],
          'method' => $method,
          'response_time' => round(microtime(TRUE) - $start, 3),
        );
        $_SESSION['mediamosafield_debug_info'] = $this->log;
      }
    }
    
    // return the result
    if ($process_result) {
      try {
        return new SimpleXMLElement($xml);
      }
      catch (Exception $e) {
        return $xml;
      }
    }
    return $xml;
  }

  protected function _request($method, $uri, $data = NULL) {
    $data_log = '';

    // build the HTTP header
    $headers = array();
    if (!is_null($this->cookie)) {
      $headers['Cookie'] = $this->cookie;
    }
    if (strcasecmp($method, 'POST') === 0) {
      $headers['Content-Type'] = 'application/x-www-form-urlencoded';
    }
    if (!is_null($data)) {
      if (is_array($data)) {
        $data_log = '('. count($data) .')';
        $data = http_build_query($data, '', '&');
      }
      else {
        $data_log = '(custom query)';
      }
    }
    $start = microtime(TRUE);
    $response = drupal_http_request(
      $this->mediamosaserver_uri . str_replace($this->mediamosaserver_uri, '', $uri), // url
      $headers, // headers
      $method, // method
      $data, // data
      0 // retry
    );
    
    // log the request
    $this->log[] = array('method' => $method . $data_log, 'uri' => $this->mediamosaserver_uri . $uri, 'response_time' => round(microtime(TRUE) - $start, 3));
    if (variable_get('video_mediamosa_debug', FALSE)) {
      $_SESSION['mediamosafield_debug_info'] = $this->log;
    }

    // check the result for success (200 == OK)
    if ((int)$response->code !== 200) {
      return FALSE;
    }
    return $response;
  }
  
  public function check_result($result, $ok = FALSE, $error = FALSE) {
    $result_id = (int)$result->header->request_result_id;
    if ($result_id != 601 && $result_id != 705) {
      if ($error) {
        $error = t($error, array('%code' => $result_id, '%message' => $result->header->request_result_description));
        drupal_set_message($error, 'error');
      }
      return FALSE;
    }
    else {
      if ($ok) {
        drupal_set_message(t($ok));
      }
      return TRUE;
    }
  }
  
  public function __call($name, $arguments = Array()) {
    $uri = str_replace('_', '/', $name);
    $this->request(
      !empty($arguments) ? $arguments[0] : 'POST',
      $uri,
      !empty($arguments) ? array_slice($arguments, 1) : null
    );
  }
}
