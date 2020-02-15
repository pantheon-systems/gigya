<?php

/**
 * Form builder for the Gigya DS administration form.
 *
 * @return array
 *   The Gigya DS administration form.
 */
function gigya_ds_admin_form() {
  $form['gigya_ds_mapping'] = array(
    '#type' => 'textarea',
    '#title' => t('Gigya DS mapping'),
    '#description' => t('Gigya DS JSON Mapping.'),
    '#size' => 60,
    '#default_value' => variable_get('gigya_ds_mapping', ''),
  );

  $form['gigya_ds_use_search'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use search.'),
    '#default_value' => variable_get('gigya_ds_use_search', false),
  );

  return system_settings_form($form);
}
