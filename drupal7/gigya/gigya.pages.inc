<?php

/**
 * @file
 * Page display functions for for Gigya module.
 */

/**
 * Page callback for the user/%user/edit/gigya page.
 *
 * see gigya_menu()
 *
 * @return null|string
 *   Gigya connect page HTML
 *
 * @throws \Exception
 */
function gigya_connect() {
  drupal_set_title(t('Edit your Social Network connections'));
  global $user;
  if ($user->uid === arg(1)) {
    return theme('gigya_connect_display');
  }
  return t('Only users can edit their own connections.');
}

/**
 * Socialize-login page callback.
 *
 * see gigya_menu()
 *
 * return array
 *   An array of ajax commands.
 */
function gigya_login_ajax() {
  require_once drupal_get_path('module', 'gigya') . '/GSSDK.php';
  ctools_include('ajax');
  ctools_include('modal');
  $params = $_POST; // check for XSS line 54.
  /* If the admin set a post login redirect send
  the user to it if no just reload the page. */
  $dest = variable_get('gigya_login_uiconfig_urlredirect', '');
  if (!empty($dest)) {
    $success_command[] = ctools_ajax_command_redirect($dest);
  }
  else {
    $success_command[] = ctools_ajax_command_reload();
  }
  // Check if the user is not logged in.
  if (!user_is_logged_in()) {
    $commands = array();
    $_SESSION['gigya_suppress_notifylogin_js'] = TRUE;
    module_load_include('inc', 'gigya');
    if (_gigya_validate_sig($params['UID'], $params['signatureTimestamp'], $params['UIDSignature'], "socialize")) {
      // Check for XSS inside object;
      gigya_filter_xss_array($params);
      $gigya_user = $params['user'];

      /* Check to see if the Gigya user is a Drupal user.
      if the uid isn't a Drupal uid then the user_load will return false. */
      if (is_numeric($gigya_user['UID']) && $params['isSiteUID'] === 'true' && $usr = user_load($gigya_user['UID'])) {
        if (!variable_get('user_email_verification', TRUE)) {
          /* We can suppress the notify login JS because the user is clearly
          already logged into gigya.
          $_SESSION['gigya_suppress_notifylogin_js'] = TRUE;
          Log the user in. */
          $form_state = array('uid' => $usr->uid);
          $form = array();
          // A small hack because on variables can be passed by reference
          user_login_submit($form, $form_state);
        }
        else {
          $commands[] = ajax_command_alert(t('You must validate your email address for this account before logging in.'));
        }
        return array('#type' => 'ajax', '#commands' => $success_command);
      }
      // If the user isn't a Drupal user, try to register them if allowed.
      elseif (variable_get('user_register', 1)) {
        $form_state['values'] = array();
        $uname = $params['user']['email'];
        $user_existes = gigya_dose_user_exist($uname);
        if ($user_existes) {
          $form_state = array(
            'title' => t('Please login to link the accounts'),
            'ajax' => TRUE,
            'email' => $params['user']['email'],
          );
          $commands = ctools_modal_form_wrapper('gigya_link_accounts_form', $form_state);
          return array('#type' => 'ajax', '#commands' => $commands);
        }
        else {
          $form_state['values']['name'] = $uname;
        }
        $form_state['values']['mail'] = $uname;
        $form_state['values']['pass'] = user_password();
        $_SESSION['gigya_uid'] = $params['user']['UID'];

        // Allow other modules to change this info before we create the user.
        drupal_alter('gigya_create_user', $form_state, $params);
        drupal_form_submit('user_register_form', $form_state);

        if (form_get_errors()) {
          if (variable_get('gigya_debug', 0)) {
            $messages = drupal_get_messages('error');
            $output = print_r($messages, TRUE);
            // We were unable to register a valid new user.
            // Send the error messages to watchdog.
            watchdog('gigya', 'Unable to auto-create user account:' . $output);
          }
          /* If there is an error in the registration process,
          return the registration form in a modal. */
          $form_state = array(
            'title' => t('Please fill-in missing details'),
            'ajax' => TRUE,
            'gigya' => TRUE,
          );
          $commands = ctools_modal_form_wrapper('user_register_form', $form_state);
          return array('#type' => 'ajax', '#commands' => $commands);
        }
        // The register process is done.
        $commands[] = $success_command;
        return array('#type' => 'ajax', '#commands' => $commands);
      }
      else {
        $commands[] = ajax_command_alert(t('Only site administrators can create new user accounts.'));
        return array('#type' => 'ajax', '#commands' => $commands);
      }
    }
    $commands[] = ajax_command_alert(t('Gigya signature not valid.'));
    return array('#type' => 'ajax', '#commands' => $commands);
  }
  $commands[] = $success_command;
  return array('#type' => 'ajax', '#commands' => $commands);
}

/**
 * RaaS login callback.
 *
 * @return array
 *   An array or ajax commands
 * @throws Exception
 */
function gigya_raas_login_ajax() {
  if (empty($_POST) || user_is_logged_in()) {
    return;
  }

  if (isset($_COOKIE['loginCount']) && $_COOKIE['loginCount'] > 2) {
    setcookie('loginCount', 0);
    return;
  }
  //Includes
  ctools_include('ajax');
  ctools_include('modal');
  module_load_include('inc', 'gigya');
  require_once drupal_get_path('module', 'gigya') . '/GSSDK.php';

  //Init params
  $commands = array();
  $_SESSION['gigya_suppress_notifylogin_js'] = TRUE;
  $params = $_POST; // Post is not displayed on screen no need to filter_xss
  $redirect_url = variable_get('raas_redirect_url', FALSE);

  $success_command = ['command' => 'gigyaLoginRedirect', 'redirectTarget' => $redirect_url];
  // Make sure the request came from Gigya.
  if (_gigya_validate_sig($params['UID'], $params['signatureTimestamp'], $params['UIDSignature'], "accounts")) {
    // Get the account from Gigya.
    $gigya_account = _get_gigya_account_from_uid($params['UID']);

    // If we get an error abort the process with a message.
    if (is_numeric($gigya_account)) {
      $msg = t("Oops! Something went wrong during your login/registration process. Please try to login/register again.");
      _gigya_set_logout_cookie();
      $commands[] = ajax_command_alert($msg);
      $commands[] = ctools_ajax_command_reload();
      $ctx = array(
        "result" => "error",
        "type" => "gigya_api_error"
      );
      drupal_alter("gigya_login_register_response", $commands, $ctx);
      return array('#type' => 'ajax', '#commands' => $commands);
    }
    // Check for XSS inside object;
    gigya_filter_xss_array($gigya_account);

    // If the user do not have email in there gigya profile abort the process with a message.
    if (empty($gigya_account['loginIDs']['emails'])) {
      _gigya_set_logout_cookie();
      $commands[] = ajax_command_alert(t('Email address is required by Drupal and is missing, please contact the site administrator.'));
      $commands[] = ctools_ajax_command_reload();
      $ctx = array(
        "result" => "error",
        "type" => "missing_email"
      );
      drupal_alter("gigya_login_register_response", $commands, $ctx);
      return array('#type' => 'ajax', '#commands' => $commands);
    }
    if (variable_get('gigyaSyncBase', 'LoginIDsEmail') == 'GigyaUID') {
      $login_id = _gigya_get_duid_by_guid($params['UID']);
    }
    else {
      // Check if users exists on the system.
      $login_id = _gigya_check_for_existing_email($gigya_account);
    }

    /* Set global variable so we would know the user as logged in
          RaaS in other functions down the line.*/
    global $_gigya_raas_login;
    $_gigya_raas_login = TRUE;

    $_SESSION['gigya_raas_uid'] = $params['UID'];

    if ($login_id !== FALSE) {
      $drupal_mode = "login";
      if (variable_get('gigyaSyncBase', 'LoginIDsEmail') == 'GigyaUID') {
        $user = user_load($login_id);
        if ($unique_email = _gigya_check_emails_uniqueness($gigya_account, $user->uid)) {
          if ($user->mail !== $unique_email) {
            $user->mail = $unique_email;
            user_save($user);
          }
        }
        else {
          _gigya_set_logout_cookie();
          $commands[] = ajax_command_alert(t('Email already exists'));
          $commands[] = ctools_ajax_command_reload();
          $ctx = array(
            "result" => "error",
            "type" => "email_already_exists"
          );
          drupal_alter("gigya_login_register_response", $commands, $ctx);
          return array('#type' => 'ajax', '#commands' => $commands);
        }
      }
      else {
        $user = user_load_by_mail($login_id);
      }

      if (gigya_user_not_has_bypass_raas($user)) {

        // A small hack because on variables can be passed by reference
        $form_state = array('uid' => $user->uid);
        $form = array();
        user_login_submit($form, $form_state);
        $edit = array();
        $user = _gigya_process_field_mappings($user, $gigya_account, $edit, 'gigya_raas_update_user');
        if (!$user) {
          session_destroy();
          $msg = t("Oops! Something went wrong during your login process. Please try to login again.");
          _gigya_set_logout_cookie();
          $commands[] = ajax_command_alert($msg);
          $commands[] = ctools_ajax_command_reload();
          $ctx = array(
            "result" => "error",
            "type" => "drupal_error",
            "drupal_mode" => $drupal_mode
          );
          drupal_alter("gigya_login_register_response", $commands, $ctx, $user, $gigya_account);
          return array('#type' => 'ajax', '#commands' => $commands);

        }
      }
      else {
        _gigya_set_logout_cookie();
        watchdog('gigya', 'Invalid login occured User with "Bypass Registration-as-a-Service" Permission Email: @email tried to login using Gigya.',
          array('@loginId' => $login_id), WATCHDOG_WARNING);
        $msg = t('Error occurred please contact site admin.');
        $commands[] = ctools_modal_command_display('An error occurred', '<div class="gigya-error">' . $msg . '</div>');
        $ctx = array(
          "result" => "error",
          "type" => "has_bypass",
          "drupal_mode" => $drupal_mode
        );
        drupal_alter("gigya_login_register_response", $commands, $ctx, $user, $gigya_account);
        return array('#type' => 'ajax', '#commands' => $commands);
      }
    }
    // If the user does not exist register him/her.
    else {
      $drupal_mode = "create";
      if (variable_get('gigyaSyncBase', 'LoginIDsEmail') == 'GigyaUID') {
        if ($unique_email = _gigya_check_emails_uniqueness($gigya_account, 0)) {
          $email = $unique_email;
        }
        else {
          _gigya_set_logout_cookie();
          $commands[] = ajax_command_alert(t('Email already exists'));
          $commands[] = ctools_ajax_command_reload();
          $ctx = array(
            "result" => "error",
            "type" => "email_already_exists"
          );
          drupal_alter("gigya_login_register_response", $commands, $ctx);
          return array('#type' => 'ajax', '#commands' => $commands);
        }
      }
      else {
        if (_check_profile_email($gigya_account['profile']['email'], $gigya_account['loginIDs']['emails'])) {
          $email = $gigya_account['profile']['email'];
        }
        else {
          $email = $gigya_account['loginIDs']['emails'][0];
        }
      }

      if (variable_get('raas_username_map', 'email') == 'username') {
        $uname = !empty($gigya_account['profile']['username']) ? $gigya_account['profile']['username'] : $gigya_account['profile']['firstName'];
        if (gigya_check_username_available($uname)) {
          $username = $uname;
        }
        else {
          // If user name is taken use first name if it is not empty.
          if (!empty($gigya_account['profile']['firstName']) && gigya_check_username_available($gigya_account['profile']['firstName'])) {
            $username = $gigya_account['profile']['firstName'];
          }
          else {
            // When all fails add unique id  to the username so we could register the user.
            $username = $uname . '_' . uniqid();
          }
        }
      }
      else {
        $username = $email;
      }
      $new_user = array(
        'name' => $username,
        'pass' => user_password(),
        'mail' => $email,
        'status' => 1,
        'init' => $email,
      );
      $user = _gigya_process_field_mappings(NULL, $gigya_account, $new_user, 'gigya_raas_create_user', TRUE);
      if (!$user) {
        session_destroy();
        $msg = t("Oops! Something went wrong during your registration process. You are registered to the site but
            not logged-in. Please try to login again.");
        _gigya_set_logout_cookie();
        $commands[] = ajax_command_alert($msg);
        $commands[] = ctools_ajax_command_reload();
        $ctx = array(
          "result" => "error",
          "type" => "drupal_error",
          "drupal_mode" => $drupal_mode
        );
        //@TODO: replace hook
        $drupal_user = NULL;
        drupal_alter("gigya_login_register_response", $commands, $ctx, $drupal_user, $gigya_account);
        return array('#type' => 'ajax', '#commands' => $commands);

      }
      else {

        user_login_finalize();
      }
    }


    if ($user !== FALSE) {
      //Map the user if needed
      gigya_raas_ext_cookie_ajax(true);
      _gigya_map_user_if_needed($user->uid, $params['UID']);

      $commands[] = $success_command;
      $ctx = array(
        "result" => "success",
        "drupal_mode" => $drupal_mode);
      drupal_alter("gigya_login_register_response", $commands, $ctx, $user, $gigya_account);
      return array('#type' => 'ajax', '#commands' => $commands);
    }
    else {
      watchdog('gigya-RaaS', "User UID=@uid logged to Gigya", array('@uid' => $params['UID']), WATCHDOG_DEBUG);
      $msg = t("Oops! Something went wrong during your login/registration process. Please try to login/register again.");
      _gigya_set_logout_cookie();
      $commands[] = ajax_command_alert($msg);
      $commands[] = ctools_ajax_command_reload();
      watchdog('gigya-RaaS', 'There was a problem registering a user');
      $ctx = array(
        "result" => "error",
        "type" => "drupal_error",
        "drupal_mode" => $drupal_mode);
      drupal_alter("gigya_login_register_response", $commands, $ctx, $user, $gigya_account);
      return array('#type' => 'ajax', '#commands' => $commands);
    }
  }
  else {
    watchdog('gigya-RaaS', 'Invalid signature check apiKey and secret');
    _gigya_set_logout_cookie();
    $commands[] = ajax_command_alert(t('Gigya signature not valid.'));
    $commands[] = ctools_ajax_command_reload();
    $ctx = array(
      "result" => "error",
      "type" => "invalid_signature",
    );
    //@TODO: replace hook.
    $user = NULL;
    $gigya_account = NULL;
    drupal_alter("gigya_login_register_response", $commands, $ctx, $user, $gigya_account);
    return array('#type' => 'ajax', '#commands' => $commands);
  }
}

function gigya_check_username_available($username) {
  return !db_query("SELECT COUNT(*) FROM {users} WHERE name = :name;", array(':name' => $username))->fetchField();
}

/**
 * @param $fields Array user fields passed by reference
 * @param $return Boolean should we return the profile2 fields
 *
 * @return array profile2 fields
 */
function gigya_remove_p2_fields(&$fields, $return) {
  $p2_fields = array();
  foreach ($fields as $raas_field => $drupal_field) {
    if (strpos($drupal_field, 'p2::') === 0) {
      if ($return) {
        $p2_fields[$raas_field] = $drupal_field;
      }
      unset($fields[$raas_field]);
    }
  }
  if ($return) {
    return $p2_fields;
  }
}

/**
 * Wrapper for the Gigya Javascript userInfo function.
 *
 * @param object $account
 *   The user object.
 *
 * @return string
 *   The user info from Gigya.
 */
function gigya_get_user($account) {
  $title = isset($account->title) ? $account->title : $account->name;
  drupal_set_title(check_plain($title));

  // Add the onload functions.
  $behavior_js = 'Drupal.behaviors.gigyaGetUserInfo = { attach: function(context, settings) { gigya.services.socialize.getUserInfo(Drupal.settings.gigya.conf, {callback:Drupal.gigya.getUserInfoCallback}); }}';
  drupal_add_js($behavior_js, array('type' => 'inline', 'scope' => JS_DEFAULT));

  $userinfo = '<div id="userinfo"></div>';
  return $userinfo;
}

/**
 * A helper method to xss filter arrays.
 *
 * @param array $array
 *   The array to filter.
 */
function gigya_filter_xss_array(&$array) {
  array_walk_recursive($array, '_gigya_filter_xss');
}

/**
 * Helper to filter items passed from array_walk_recursive().
 *
 * @param string $item
 *   The array value.
 * @param string|int $key
 *   The array key.
 */
function _gigya_filter_xss(&$item, $key) {
  $item = filter_xss($item);
}


/**
 * AJAX registering callback.
 *
 * @param array $form
 *   The form.
 * @param array $form_state
 *   From state.
 *
 * @return array
 *   An array of ajax commands.
 */
function gigya_register_ajax($form, $form_state) {
  ctools_include('ajax');
  ctools_include('modal');
  ctools_add_js('ajax-responder');
  drupal_process_form('user_register_form', $form, $form_state);
  $commands[] = ctools_modal_command_dismiss();
  $dest = variable_get('gigya_login_uiconfig_urlredirect', '');
  if (!empty($dest)) {
    $commands[] = ctools_ajax_command_redirect($dest);
  }
  else {
    $commands[] = ctools_ajax_command_reload();
  }
  return array('#type' => 'ajax', '#commands' => $commands);
}

function _gigya_process_field_mappings($drupal_user, $gigya_account, $edit, $hook_name, $replace_user = FALSE) {
  $additional_user_fields = _gigya_raas_get_extra_mappings();
  if (!empty($additional_user_fields)) {
    if (module_exists("profile2")) {
      $p2_fields = gigya_remove_p2_fields($additional_user_fields, TRUE);
    }
    else {
      gigya_remove_p2_fields($additional_user_fields, FALSE);
    }
    _process_field_map($additional_user_fields, $gigya_account, $edit);
  }
  try {
    if ($drupal_user !== NULL) {
      drupal_alter($hook_name, $edit, $drupal_user, $gigya_account);
    }
    else {
      drupal_alter($hook_name, $edit, $gigya_account);
    }
    if ($replace_user) {
      global $user;
      $user = user_save(NULL, $edit);
      $drupal_user = $user;
    }
    else {
      user_save($drupal_user, $edit);
    }
    if (!empty($p2_fields)) {
      foreach ($p2_fields as $raas_field => $p2field) {
        $parts = explode('::', $p2field);
        $raas_field_parts = explode(".", $raas_field);
        $val = gigya_get_nested_value($gigya_account, $raas_field_parts);
        $val = drupal_strlen($val) > $parts[3] ? drupal_substr($val, 0, $parts[3]) : $val;
        $bundles[$parts[1]][$parts[2]] = $val;
      }
      $profs = profile2_load_by_user($drupal_user);
      foreach ($bundles as $profile => $field) {
        if (isset($profs[$profile])) {
          $prof = $profs[$profile];
        }
        else {
          $prof = profile2_create(array(
            'type' => $profile,
            'uid' => $drupal_user->uid
          ));
        }
        foreach ($field as $p2_field => $val) {
          $prof->{$p2_field}[LANGUAGE_NONE][0]['value'] = $val;
        }
        drupal_alter('gigya_raas_update_profile2', $prof, $gigya_account);
        profile2_save($prof);
      }
    }
    return $drupal_user;
  } catch (Exception $e) {
    return false;
  }
}


function _gigya_raas_get_extra_mappings() {
  $additional_user_fields = variable_get('gigya_raas_extra_mappings', array());
  drupal_alter('gigya_raas_extra_mappings', $additional_user_fields);
  return $additional_user_fields;
}


function _get_gigya_account_from_uid($uid) {
  $req_params = array(
    'UID' => $uid,
    'include' => 'loginIDs,profile,data',
    'extraProfileFields' => 'username,languages,address,phones,education,honors,publications,patents,
          certifications,professionalHeadline,bio,industry,specialties,work,skills,religion,politicalView,
          interestedIn,relationshipStatus,hometown,favorites,followersCount,followingCount,locale,verified,timezone,likes'
  );
  return _get_gigya_account($req_params);
}

function gigya_raas_profile_edit_ajax() {
  global $user;
  $edit = array();
  $gigya_data = $_POST['gigyaData'];
  if (_gigya_validate_sig($gigya_data['UID'], $gigya_data['signatureTimestamp'], $gigya_data['UIDSignature'], "accounts")) {
    $gigya_account = _get_gigya_account_from_uid($gigya_data['UID']);
    $user = user_load($user->uid);
    $user = _gigya_process_field_mappings($user, $gigya_account, $edit, 'gigya_raas_update_user');
  }
  $commands = array();
  ctools_include('ajax');
  $ctx = array(
    "result" => "success",
    "drupal_mode" => "update",
  );
  drupal_alter("gigya_update_response", $commands, $ctx);
  return array('#type' => 'ajax', '#commands' => $commands);
}

function _gigya_cast_value($val, $type) {
  $type = strtolower($type);
  if ($type == "decimal" || $type == "integer" || $type == "boolean" || $type == "date" || $type == "datestamp") {
    switch ($type) {
      case 'date':
        if (is_numeric($val)) {
          $val = format_date(intval($val / 1000), 'custom', 'Y-m-d H:i:s', 'UTC');
        }
        else {
          if (strpos($val, "Z") !== FALSE) {
            $val = substr($val, 0, -5);
          }
        }
        break;
      case 'datestamp':
        if (is_numeric($val)) {
          $val = intval($val / 1000);
        }
        else {
          $val = strtotime($val);
        }
        break;

      case 'integer':
        $val = intval($val);
        break;
      case 'decimal':
        $val = floatval($val);
        break;
      case 'boolean':
        $val = filter_var($val, FILTER_VALIDATE_BOOLEAN) ? 1 : 0;
        break;
    }
  }
  else {
    $val = drupal_strlen($val) > $type ? drupal_substr($val, 0, $type) : $val;
  }
  return $val;
}


function _process_field_map($additional_user_fields, $gigya_account, &$edit) {
  foreach ($additional_user_fields as $raas_field => $drupal_field) {
    $raas_field_parts = explode(".", $raas_field);
    $drupal_field_parts = explode("::", $drupal_field);
    $val = gigya_get_nested_value($gigya_account, $raas_field_parts);
    $val = _gigya_cast_value($val, $drupal_field_parts[1]);
    $edit[$drupal_field_parts[0]][LANGUAGE_NONE][0]['value'] = $val;
  }
}
