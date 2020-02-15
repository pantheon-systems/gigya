<?php

/**
 * Form builder for the Gigya Delete administration form.
 *
 * @return array
 *   The Gigya Delete administration form.
 */
function gigya_delete_admin_form() {

  $form['gigya_delete_enable'] = ['#type' => 'checkbox',
    '#default_value' => variable_get('gigya_delete_enable', 0),
    '#title' => t('Enable')];

  $form['gigya_delete_mode'] = [
    '#type' => 'select',
    '#options' => [
      'soft' => t('Deletion notification'),
      'hard' => t('Full user deletion'),
    ],
    '#default_value' => variable_get('gigya_delete_mode', 'soft'),
    '#title' => t('Action'),
    '#description' => t('Full user deletion means all the data associated with this user will be permanently deleted from Drupal\'s database.<br>  
Deletion notification means that the user is not deleted, but an indication is added that they were deleted from Gigya.<br> In this case, you should usually add custom code to handle the user data in Drupal\'s database. For more information, see Gigya\'s Developer Guide.')
  ];

  $form['gigya_delete_frequency'] = ['#type' => 'textfield',
    '#default_value' => variable_get('gigya_delete_frequency', 60),
    '#title' => t('Job frequency (minutes)')];

  $form['gigya_delete_success_email'] = ['#type' => 'textfield',
    '#default_value' => variable_get('gigya_delete_success_email', ''),
    '#title' => t('Email on success'),
    '#description' => t('A comma-separated list of email that will be notified when the job completes successfully')
  ];

  $form['gigya_delete_fail_email'] = ['#type' => 'textfield',
    '#default_value' => variable_get('gigya_delete_fail_email', ''),
    '#title' => t('Email on failure'),
    '#description' => t('A comma-separated list of email that will be notified when the job fails or completes with errors')
  ];

  $form['gigya_delete_s3_bucket'] = [
    '#type' => 'textfield',
    '#default_value' => variable_get('gigya_delete_s3_bucket', ''),
    '#title' => t('Bucket name')
  ];

  $form['gigya_delete_s3_access_key'] = [
    '#type' => 'textfield',
    '#default_value' => variable_get('gigya_delete_s3_access_key', ''),
    '#title' => t('Access key')
  ];

  $description = t('Specify the Gigya Secret (Base64 encoded) key for this domain');
  $secret_key = gigya_decrypt_value(variable_get('gigya_delete_s3_secret_key', ''));
  if (strlen($secret_key) > 0) {
    $description .= ", current key first and last letters are " . substr($secret_key, 0, 2) . "****" . substr($secret_key, strlen($secret_key) - 2, 2);
  }

  $form['gigya_delete_s3_secret_key'] = [
    '#type' => 'password',
    '#default_value' => $secret_key,
    '#title' => t('Secret key'),
    '#description' => $description,
  ];

  $form['gigya_delete_s3_object_key_prefix'] = [
    '#type' => 'textfield',
    '#default_value' => variable_get('gigya_delete_s3_object_key_prefix', ''),
    '#title' => t('Object key prefix')
  ];


  return system_settings_form($form);
}

function gigya_delete_admin_form_validate($form, &$form_state) {
  $_secret = $form_state['values']['gigya_delete_s3_secret_key'];
  if ($_secret) {
    $form_state['values']['gigya_delete_s3_secret_key'] = gigya_encrypt_value($_secret);
  }
  else {
    $form_state['values']['gigya_delete_s3_secret_key'] = variable_get('gigya_delete_s3_secret_key', '');
  }
}
