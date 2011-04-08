<?php
// $Id$
/**
 * @file
 * synetic_video widget hooks and callbacks.
 */

/**
 * Implementation of CCK's hook_widget_settings($op = 'form').
 */
function video_mediamosa_upload_widget_settings_form($widget) {
  $form = module_invoke('filefield', 'widget_settings', 'form', $widget);

  if ($form['file_extensions']['#default_value'] == 'txt') {
    $form['file_extensions']['#default_value'] = 'mp4 mpeg avi mpg wmv flv mov';
  }
  
  //default settings
  $default = video_default_widget_settings($widget);
  $form = $form + $default;
  
  //Remove options that are configured in the video_mediamosa transcoder
  unset($form['path_settings']);
  unset($form['plugins']['autoconversion']);
  unset($form['default']);
  unset($form['default_video_thumb']);
  
  
  
  return $form;
}

/**
 * Implementation of CCK's hook_widget_settings($op = 'save').
 */
function video_mediamosa_upload_widget_settings_save($widget) {
  $filefield_settings = module_invoke('filefield', 'widget_settings', 'save', $widget);
  return array_merge($filefield_settings, array('default_dimensions', 'default_player_dimensions'));
}

/**
 * Element #value_callback function.
 */
function video_mediamosa_upload_widget_value($element, $edit = FALSE) {
  if (!$edit) {
    $file = field_file_load($element['#default_value']['fid']);
    $item = $element['#default_value'];
  }
  else {
    $field = content_fields($element['#field_name'], $element['#type_name']);
    
    $item        = $edit;
    $item['fid'] = $element['#post'][$element['#field_name']][$element['#delta']]['fid'];
    
    // Load file if the FID has changed so that it can be saved by CCK.
    $file = field_file_load($item['fid']);

    // If the file entry doesn't exist, don't save anything.
    if (empty($file)) {
      $item = array();
    }

    // Checkboxes loose their value when empty.
    // If the list field is present make sure its unchecked value is saved.
    if (!empty($field['list_field']) && empty($edit['list'])) {
      $item['list'] = 0;
    }
  }
  // Merge file and item data so it is available to all widgets.
  if (isset($item['data']) && isset($file['data'])) {
    $file['data'] = array_merge($item['data'], $file['data']);
  }
  $item = array_merge($item, $file);

  return $item;
}

/**
 * Element #process callback function.
 */
function video_mediamosa_upload_widget_process($element, $edit, &$form_state, $form) {
  $path        = drupal_get_path('module', 'video_mediamosa_upload');
  
  drupal_add_js($path . '/javascript/video_mediamosa_upload.js');
  drupal_add_css($path  . '/css/video_mediamosa_upload.css');
  
  //Use filefield presentation and overwrite only what is necessary.
  filefield_widget_process($element, $edit, $form_state, $form);
  
  //Remove ahah handler
  $element['filefield_upload'] = Array(
    '#type'   => 'submit',
    '#value'  => t('Upload'),
    '#attributes' => array(
      'class' => 'video_mediamosa_upload_direct'
    )
  );
  //Add class for javascript to recognise the element.
  $element['upload']['#attributes'] = array(
    'class' => 'video_mediamosa_upload'
  );
  
  if (!empty($element['#value']['fid'])) {
    $element['data']['title'] = array(
      '#id'             => $element['#id'] . '-data-title',
      '#name'           => $element['#name'] . '[data][title]', 
      '#type'           => 'textfield',
      '#title'          => t('Titel'),
      '#default_value'  => $element['#value']['fid'] ? $element['#value']['data']['title'] : '',
    );
    
    $element['data']['description'] = array(
      '#id'             => $element['#id'] . '-data-description',
      '#name'           => $element['#name'] . '[data][description]', 
      '#type'           => 'textarea',
      '#title'          => t('Beschrijving'),
      '#default_value'  => $element['#value']['fid'] ? $element['#value']['data']['description'] : '',
    );
    
    //Thumbnail selection
    $video_mediamosa = new video_mediamosa();
    $video = $video_mediamosa->load($element['#value']['fid']);
    $thumb_options = array();
    
    //Video has thumbnails, add the form field
    if (!empty($video->thumbnails)) {
      //Build the options list
      foreach ($video->thumbnails AS $thumb_fid) {
        $thumb_file = field_file_load($thumb_fid);
        if (module_exists('imagecache')) {
          $thumb_options[$thumb_fid] = theme('imagecache', 'video_mediamosa_thumbchoice', $thumb_file['filepath'], 'test', 'tset');
        }
        else {
          $thumb_options[$thumb_fid] = theme('image', $thumb_file['filepath'], 'test', 'tset');
        }
      }
      
      $element['data']['thumbnail'] = array(
        '#id'             => $element['#id'] . '-data-thumbnail',
        '#name'           => $element['#name'] . '[data][thumbnail]', 
        '#type'           => 'radios',
        '#options'        => $thumb_options,
        '#title'          => t('Thumbnail selectie'),
        '#default_value'  => $element['#value']['data']['thumbnail'],
      	'#attributes'     => array(
          'class' => 'video_mediamosa_upload-thumbnail-selection',
        )
      );
    }
  }
  
  return $element;
}

/**
 * Remove all ahah references to prevent uploading when adding
 * another item.
 * Also adds a rel attribute to assist identification from javascript.
 * 
 * @param array $form
 * @param array $form_state
 * @param array $form_id
 */
function video_mediamosa_upload_form_alter(&$form, $form_state, $form_id) {
  //Shortcut for better readability
  $element =& $form['field_movement_video']['field_movement_video_add_more'];
  
  unset($element['#ahah']);
  $element['#attributes']['rel'] = $element['#type_name'] . '/' . $element['#field_name'];
}