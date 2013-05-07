<?php

/**
 * Menu callback: content administration.
 */
function inm_drupal_admin_list($form, $form_state) {
  $form['admin'] = inm_drupal_admin_topics();
  return $form;
}

/**
 * Form builder: Builds the node administration overview.
 */
function inm_drupal_admin_topics() {
  // Build the sortable table header.
  $header = array(
    'title' => array('data' => t('Title'), 'field' => 'title', 'sort' => 'asc'),
    'description' => t('Description'),
    'status' => array('data' => t('Status'), 'field' => 'status'),
    'version' => array('data' => t('Updated'), 'field' => 'version'),
    'operations' => t('Operations'),
  );

  $query = db_select('inm_topics', 't')->extend('PagerDefault')->extend('TableSort');

  $topics = $query
    ->fields('t',array('tid', 'title', 'description', 'status', 'version'))
    ->limit(50)
    ->orderByHeader($header)
    ->execute()
    ->fetchAll();
  
  // Prepare the list of topics.
  $destination = drupal_get_destination();
  $options = array();
  $selection = array();
  foreach ($topics as $topic) {
    $operations = array(
      l(t('edit permissions'), 'admin/config/services/inm_drupal/topic/'. $topic->tid .'/permissions'),    
    );
    $options[$topic->tid] = array(
      'title' => check_plain($topic->title),
      'description' => check_plain($topic->description),
      'status' => $topic->status ? t('enabled') : t('disabled'),
      'version' => $topic->version,
      'operations' => implode(' ', $operations), 
    );
    $selection[$topic->tid] = $topic->status;
  }
  
  // Only use a tableselect when the current user is able to perform any
  // operations.
  $form['topics'] = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $options,
    '#empty' => t('No content available.'),
    '#default_value' => $selection,
  );

  $form['pager'] = array('#markup' => theme('pager'));
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
    '#submit' => array('inm_drupal_admin_topics_submit'),
  );
  
  return $form;
}

function inm_drupal_admin_topics_submit($form, &$form_state) {
  $enabled_num = $disabled_num = 0;
  $topics = $form_state['values']['topics'];
  
  // Enable checked topics 
  $enable = array_filter($topics);
  if (!empty($enable)) {
    inm_drupal_topics_update_status(1, array_keys($enable));
  }
  
  // Disable unchecked topics
  $disable = array_diff_assoc($topics, $enable);
  if (!empty($disable)) {
    inm_drupal_topics_update_status(0, array_keys($disable));
  }
  
  drupal_set_message(t('Topic updated.'));  
}

/**
 * Page callback for "Refresh topics"
 */
function inm_drupal_admin_topics_refresh() {
  inm_drupal_topics_fetch_all();
  
  drupal_set_message(t('Topics refreshed'));
  
  drupal_goto('admin/config/services/inm_drupal');
} 

/**
 * Implements hook_admin_settings() for module settings configuration.
 */
function inm_drupal_admin_settings_form($form_state) {
  $form['inm_drupal_site_key'] = array(
    '#title' => t('Site API Key'),
    '#type' => 'textfield',
    '#default_value' => variable_get('inm_drupal_site_key', ''),
    '#size' => 60,
    '#maxlength' => 120,
    '#required' => TRUE,
    '#description' => t('This ID is unique to each site and can be found on inline manual portal.'),
  );

  return system_settings_form($form);
}

/**
 * Page callback for topic permissions editing 
 */
function inm_drupal_admin_topic_permissions_form($form, &$form_state, $tid) {
  $topic = inm_drupal_topic_load($tid);
  
  // Retrieve role names for columns.
  $role_names = user_roles();

  // Fetch permissions for all roles or the one selected role.
  $role_permissions = user_role_permissions($role_names);
  
  // Store $role_names for use when saving the data.
  $form['role_names'] = array(
    '#type' => 'value', 
    '#value' => $role_names,
  );
  
  $options = array();
  $hide_descriptions = system_admin_compact_mode();
  
  if ($permissions = inm_drupal_permission()) {
    foreach ($permissions as $perm => $perm_item) {
      if ($perm == 'view inm_drupal_topic_'. $tid) {
        // Fill in default values for the permission.
        $perm_item += array(
          'description' => '', 
          'restrict access' => FALSE, 
          'warning' => !empty($perm_item['restrict access']) ? t('Warning: Give to trusted roles only; this permission has security implications.') : '',
        );
        $options[$perm] = '';
        $form['permission'][$perm] = array(
          '#type' => 'item', 
          '#markup' => $perm_item['title'], 
          '#description' => theme('user_permission_description', array('permission_item' => $perm_item, 'hide' => $hide_descriptions)),
        );
        foreach ($role_names as $rid => $name) {
          // Builds arrays for checked boxes for each role
          if (isset($role_permissions[$rid][$perm])) {
            $status[$rid][] = $perm;
          }
        }
      }
    }
  }
  
  // Have to build checkboxes here after checkbox arrays are built
  foreach ($role_names as $rid => $name) {
    $form['checkboxes'][$rid] = array(
      '#type' => 'checkboxes', 
      '#options' => $options, 
      '#default_value' => isset($status[$rid]) ? $status[$rid] : array(), 
      '#attributes' => array('class' => array('rid-' . $rid)),
    );
    $form['role_names'][$rid] = array(
      '#markup' => check_plain($name),
      '#tree' => TRUE,
    );
  }
  
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save permissions'),
  );

  $form['#attached']['js'][] = drupal_get_path('module', 'user') . '/user.permissions.js';
  
  return $form;
}

function inm_drupal_admin_topic_permissions_form_submit($form, &$form_state) {
  module_load_include('inc', 'user', 'user.admin');
  user_admin_permissions_submit($form, $form_state);
}

function theme_inm_drupal_admin_topic_permissions_form($variables) {
  if (isset($variables['form']['permission']) && count($variables['form']['permission'])) {
    return theme('user_admin_permissions', $variables);
  } else {
    return t('There are no topics associated with this ID.');
  }
} 
