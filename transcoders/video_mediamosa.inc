<?php
// $Id$
/**
 * @file
 * Transcoder class file to handle ffmpeg settings and conversions.
*/


class video_mediamosa implements transcoder_interface {
  // Naming for our radio options.  Makes it easy to extend our transcoders.
  private $name    = 'Mediamosa';
  private $value   = 'video_mediamosa';
  private $is_wsod = TRUE; // web service on demand service

  protected $mediamosa_client;
  
  public function __construct() {
    $this->mediamosa_client = mediamosa_client_get_client();
  }
  
  /**
   * Not implemented for this transcoder
   * @see sites/all/modules/video/includes/transcoder_interface#run_command()
   */
  public function run_command($options) { return; }

  /**
   * Not implemented for this transcoder
   * @see sites/all/modules/video/includes/transcoder_interface#run_command()
   */
  public function generate_thumbnails($video) { }

  /**
   * Not implemented for this transcoder
   * @see sites/all/modules/video/includes/transcoder_interface#run_command()
   */
  public function convert_video($video) { }

  /**
   * Not implemented for this transcoder
   * @see sites/all/modules/video/includes/transcoder_interface#run_command()
   */
  public function video_converted_extension() {
    return 'flv';
  }
  
  /**
   * Not implemented for this transcoder
   * @see sites/all/modules/video/includes/transcoder_interface#run_command()
   */
  public function create_job($video) {
    
  }

  /**
   * Not implemented for this transcoder
   * @see sites/all/modules/video/includes/transcoder_interface#run_command()
   */
  public function update_job($video) {
    
  }
  
  /**
   * Not implemented for this transcoder
   * @see sites/all/modules/video/includes/transcoder_interface#run_command()
   */
  public function delete_job($video) {
    
  }
  
  /**
   * Forward to load method.
   * 
   * @see sites/all/modules/video/includes/transcoder_interface#run_command()
   */
  public function load_job($fid) {
    return $this->load($fid);
  }
  
  /**
   * Not implemented for this transcoder
   * @see sites/all/modules/video/includes/transcoder_interface#run_command()
   */
  public function load_job_queue() {
    
  }
  
  /**
   * Not implemented for this transcoder
   * @see sites/all/modules/video/includes/transcoder_interface#run_command()
   */
  public function load_completed_job($video) {
    
  }
  
  /**
   * Not implemented for this transcoder
   * @see sites/all/modules/video/includes/transcoder_interface#run_command()
   */
  public function change_status($vid, $status) {
    
  }
  
  /**
   * Not YET implemented for this transcoder
   * @see sites/all/modules/video/includes/transcoder_interface#run_command()
   */
  public function get_playtime($video) {
    return 0;
  }

  /**
   * Not YET implemented for this transcoder
   * @see sites/all/modules/video/includes/transcoder_interface#run_command()
   */
  public function get_dimensions($video) {
    return Array(
      'width'   => 0,
      'height'  => 0
    );
  }

  /**
   * Interface Implementations
   * @see sites/all/modules/video/includes/transcoder_interface#get_name()
   */
  public function get_name() {
    return $this->name;
  }

  /**
   * Interface Implementations
   * @see sites/all/modules/video/includes/transcoder_interface#get_value()
   */
  public function get_value() {
    return $this->value;
  }

  /**
   * Interface Implementations
   * @see sites/all/modules/video/includes/transcoder_interface#get_help()
   */
  public function get_help() {
    return l(t('Mediamosa Online Manual'), 'http://www.mediamosa.org');
  }

  /**
   * Interface Implementations
   * @see sites/all/modules/video/includes/transcoder_interface#get_help()
   */
  public function is_wsod() {
    return $this->is_wsod;
  }
  
  /**
   * Interface Implementations
   * @see sites/all/modules/video/includes/transcoder_interface#admin_settings()
   */
  public function admin_settings() {
    $form                       = array();
    $max_result_set             = 200;
    $transcode_profiles         = $this->getTranscodeProfiles();
    $enabled_transcode_profiles = array_filter((array)variable_get('video_mediamosa_transcode', ''));
    
    //Limit main profile selection to enabled profiles
    if (!empty($enabled_transcode_profiles)) {
      foreach ($enabled_transcode_profiles AS $profile_id => $profile) {
        $enabled_transcode_profiles[$profile_id] = $transcode_profiles[$profile_id];
      }
    }
    //No profiles selected yet, display all options
    else {
      $enabled_transcode_profiles = $transcode_profiles;
    }
    
    $form['video_mediamosa_start'] = array(
      '#type' => 'markup',
      '#value' => '<div id="video_mediamosa">',
    );
    
    $form['connection'] = array(
      '#type' => 'fieldset',
      '#title' => t('Connection settings'),
    );
    if (function_exists('madrest_get_uri_definition')) {
      $form['connection']['video_mediamosa_use_rest_interface'] = array(
        '#type' => 'checkbox',
        '#title' => t('Use the MediaMosa REST interface'),
        '#description' => t('Unchecking this item requires a working copy of MediaMosa at this instance of Drupal.'),
        '#default_value' => variable_get('video_mediamosa_use_rest_interface', TRUE),
      );
      $form['connection']['video_mediamosa_masquerade_owner'] = array(
        '#type' => 'checkbox',
          '#title' => t("Always use the owner's credentials to perform an action on an item."),
        '#description' => t('Use this option with care!'),
        '#default_value' => variable_get('video_mediamosa_masquerade_owner', FALSE),
      );
    }
    $form['connection']['video_mediamosa_username'] = array(
      '#type' => 'textfield',
      '#title' => t('Username'),
      '#default_value' => variable_get('video_mediamosa_username', ''),
    );
    $form['connection']['video_mediamosa_password'] = array(
      '#type' => 'textfield',
      '#title' => t('Password'),
      '#default_value' => variable_get('video_mediamosa_password', ''),
    );
    $form['connection']['video_mediamosa_uri'] = array(
      '#type' => 'textfield',
      '#title' => t('URL'),
      '#default_value' => variable_get('video_mediamosa_uri', ''),
    );
    $form['connection']['video_mediamosa_app_id'] = array(
      '#type' => 'textfield',
      '#title' => t('app id'),
      '#default_value' => variable_get('video_mediamosa_app_id', ''),
    );
    $form['encoding'] = array(
      '#type' => 'fieldset',
      '#title' => t('Transcoding settings'),
    );
    if (!empty($transcode_profiles)) {

      $form['encoding']['video_mediamosa_transcode'] = array(
        '#type' => 'checkboxes',
        '#options' => $transcode_profiles,
        '#multiple' => TRUE,
        '#size' => count($transcode_profiles),
        '#description' => t('Which encoding profiles to enable. The video is transcoded with these profiles and are each available for playing after encoding'),
        '#title' => t('Video transcode profiles'),
        '#default_value' => variable_get('video_mediamosa_transcode', ''),
      );
      $form['encoding']['video_mediamosa_transcode_main_profile'] = array(
        '#type' => 'select',
        '#options' => $enabled_transcode_profiles,
        '#required' => FALSE,
        '#description' => t('Select which profile is the main encoding profile, the video module will display this video by default'),
        '#title' => t('Main transcode profile'),
        '#default_value' => variable_get('video_mediamosa_transcode_main_profile', ''),
      );
      if (empty($enabled_transcode_profiles)) {
        $form['encoding']['video_mediamosa_transcode_main_profile']['#disabled'] = TRUE;
      }
    }
    else {
      $form['encoding']['message'] = Array(
        '#type'  => 'item',
        '#value' => t('No transcoding profiles found. Note: transcoding profiles are only available with valid MediaMosa credientials.')
      );
    }
    
    $form['storage'] = array(
      '#type' => 'fieldset',
      '#title' => t('Storage settings'),
    );
    $form['storage']['video_mediamosa_download_videos'] = array(
      '#type'          => 'checkbox',
      '#title'         => t('Download the video\'s to the local drupal install'),
      '#description'   => t('The video\'s will be downloaded and when played streamen from the local server.'.
                        '<br />WARNING: turning off this feature requires the mediamosa server to be configured for cross site flash'),
      '#default_value' => variable_get('video_mediamosa_download_videos', '1'),
      '#disabled'      => TRUE
    );
    $form['storage']['video_mediamosa_keep_source'] = array(
      '#type' => 'checkbox',
      '#title' => t('Keep the source files after encoding'),
      '#description' => t('Disabling this feature causes the source files to be deleted after succesfully encoding the video'),
      '#default_value' => variable_get('video_mediamosa_keep_source', 1),
    );
    $form['storage']['video_mediamosa_remove_downloaded_asset'] = array(
      '#type' => 'checkbox',
      '#title' => t('Remove the asset from the server after the files are succesfully downloaded'),
      '#description' => t('Disabling this feature causes the files to stay on the mediamosa server.'),
      '#default_value' => variable_get('video_mediamosa_remove_downloaded_asset', 0),
    );
    $form['storage']['video_mediamosa_files_path'] = array(
      '#type' => 'textfield',
      '#title' => t('Filepath (without leading or trailing /)'),
      '#description' => t('Supply a relative path (without leading /, WITH trailing /) to store all videos in a seperate directory'),
      '#default_value' => variable_get('video_mediamosa_files_path', 'videos'),
    );
    $form['storage']['video_mediamosa_files_still_path'] = array(
      '#type' => 'textfield',
      '#title' => t('Filepath for stills (without leading or trailing /)'),
      '#description' => t('Supply a relative path (without leading /, WITH trailing /) to store all stills in a seperate directory'),
      '#default_value' => variable_get('video_mediamosa_files_still_path', 'videos/stills'),
    );
    
    $form['stills'] = array(
      '#type' => 'fieldset',
      '#title' => t('Still settings'),
    );
    $form['stills']['video_mediamosa_still_type'] = array(
      '#type' => 'select',
      '#options' => Array(
        "NONE"    => "NONE - default transcoding profile behavior",
        "PICTURE" => "PICTURE - create single still",
        "NORMAL"  => "NORMAL - x stills per mediafile",
        "SECOND"  => "SECOND - one still every x seconds",
        "SCENE"   => "SCENE - one still for each scene"
      ),
      '#description' => t('Type of still to create'),
      '#title' => t('Still type'),
      '#default_value' => variable_get('video_mediamosa_still_type', ''),
    );
    
    $form['stills']['video_mediamosa_still_startframe'] = array(
      '#type' => 'textfield',
      '#description' => t('For NORMAL and SECOND stills'),
      '#size' => 50,
      '#title' => t('Still startframe'),
      '#default_value' => variable_get('video_mediamosa_still_startframe', ''),
    );
    $form['stills']['video_mediamosa_still_endframe'] = array(
      '#type' => 'textfield',
      '#description' => t('For NORMAL and SECOND stills'),
      '#size' => 50,
      '#title' => t('Still end frame'),
      '#default_value' => variable_get('video_mediamosa_still_endframe', ''),
    );
    $form['stills']['video_mediamosa_still_every_second'] = array(
      '#type' => 'textfield',
      '#description' => t('for SECOND stills'),
      '#size' => 50,
      '#title' => t('Create still each x seconds'),
      '#default_value' => variable_get('video_mediamosa_still_every_second', ''),
    );
    $form['stills']['video_mediamosa_num_still_per_mediafile'] = array(
      '#type' => 'textfield',
      '#description' => t('for NORMAL stills'),
      '#size' => 50,
      '#title' => t('Number of stills per mediafile'),
      '#default_value' => variable_get('video_mediamosa_num_still_per_mediafile', ''),
    );
    
    $form['other'] = array(
      '#type' => 'fieldset',
      '#title' => t('Other settings'),
    );
    $form['other']['video_mediamosa_debug'] = array(
      '#type' => 'checkbox',
      '#title' => t('Show MediaMosa request information'),
      '#default_value' => variable_get('video_mediamosa_debug', FALSE),
    );
    
    $form['video_mediamosa_end'] = array(
      '#type' => 'markup',
      '#value' => '</div>',
    );
    return $form;
  }

  /**
   * Validate settings form (validates MediaMosa credentials) 
   */
  public function admin_settings_validate($form, &$form_state) {
    //If REST is used, validate the credentials.
    if (!isset($form_state['values']['video_mediamosa_use_rest_interface']) || $form['values']['connection']['video_mediamosa_use_rest_interface'] == 1) {
      $mediamosa_client = new mediamosa_client();
      $mediamosa_client->set_mediamosa_server_uri($form_state['values']['video_mediamosa_uri']);
      $mediamosa_client->set_auth_credentials($form_state['values']['video_mediamosa_username'], $form_state['values']['video_mediamosa_password']);
      $result = $mediamosa_client->request('GET', '/asset?limit=1');
      
      if ($result === FALSE) {
        drupal_set_message(t('Cannot login to the Mediamosa platform with the provided credentials'), 'error');
      }
      else {
        drupal_set_message(t('Mediamosa platform credentials verified'), 'status');
      }
    }
    if (!empty($form_state['values']['video_mediamosa_still_type']) && 
       $form_state['values']['video_mediamosa_still_type'] != 'NONE') {
         
      if (!isset($form_state['values']['video_mediamosa_still_startframe'])) {
            form_set_error('video_mediamosa_still_startframe', t('Still startframe is required when selecting a still type'));
      }
      if (empty($form_state['values']['video_mediamosa_still_endframe'])) {
            form_set_error('video_mediamosa_still_endframe', t('Still endframe is required when selecting a still type'));
      }
      
      switch ($form_state['values']['video_mediamosa_still_type']) {
        case "NORMAL":
          if (empty($form_state['values']['video_mediamosa_num_still_per_mediafile'])) {
            form_set_error('video_mediamosa_num_still_per_mediafile', t('Stills per mediafile is required when selecting the still type `NORMAL`'));
          }
          break;
        case "SECOND":
          if (empty($form_state['values']['video_mediamosa_still_every_second'])) {
            form_set_error('video_mediamosa_still_every_second', t('Stills every x seconds is required when selecting the still type `SECOND`'));
          }
          break;
      }
    }
    $path = file_directory_path() . $form_state['values']['video_mediamosa_files_path'];
    if (!empty($form_state['values']['video_mediamosa_files_path']) &&
     !file_check_path($path) &&
     $this->createPath($form_state['values']['video_mediamosa_files_path']) == FALSE) {
      form_set_error('video_mediamosa_files_path', t('Cannot create directory for video path'));
    }
    
    $path = file_directory_path() . $form_state['values']['video_mediamosa_files_still_path'];
    if (!empty($form_state['values']['video_mediamosa_files_still_path']) &&
     !file_check_path($path) &&
     $this->createPath($form_state['values']['video_mediamosa_files_still_path']) == FALSE) {
      form_set_error('video_mediamosa_files_path', t('Cannot create directory for still path'));
    }
    return;
  }
  
  protected function createPath($path) {
    //Path doens't exist.
    $path_parts = explode('/', $path);
    $path       = $_SERVER['DOCUMENT_ROOT'] . base_path() . file_directory_path();
    foreach ($path_parts AS $path_part) {
      $path .= '/' . $path_part;
      if (!is_dir($path)) {
       if (!(@mkdir($path))) {
         return FALSE;
       }
       else {
         drupal_set_message(t('Successfully created directory !directory', Array('!directory' => $path)), 'status');
       }
      }
    }
    return TRUE;
  }
  
  protected function getTranscodeProfiles() {
    $response = $this->mediamosa_client->request('GET', '/transcode/profile');
    $profiles = array();
    if (!empty($response)) {
      $items = !is_array($response->items->item) 
        ? array($response->items->item)
        : $response->items->item;
      
      foreach ($response->items->item AS $item) {
        $profiles[(int)$item->profile_id] = (string)$item->profile;
      }
    }
    return $profiles;
  }

  public function processQueue() {
    //Check for video's in the queue and process them
    $result = db_query("SELECT
      {files}.fid,
      {files}.uid,
      {files}.filename,
      {files}.filepath,
      {files}.filemime,
      {files}.filesize,
      {files}.status AS file_status,
      {files}.timestamp,
      {video_mediamosa}.asset_id,
      {video_mediamosa}.src_mediafile_id,
      {video_mediamosa}.created,
      {video_mediamosa}.updated,
      {video_mediamosa}.status
    FROM {video_mediamosa} LEFT JOIN {files} USING(fid)
    WHERE
      {video_mediamosa}.status = %d",
      VIDEO_MEDIAMOSA_STATUS_QUEUED);
     
    //loop and upload
    while ($file = db_fetch_object($result)) {
      $file->status = VIDEO_MEDIAMOSA_STATUS_UPLOADING;
      $this->update($file);
      
      $this->upload($file);
      
      $file->status = VIDEO_MEDIAMOSA_STATUS_ENCODING;
      $this->update($file);
    }
  }
  
  /**
   * Download videos to the local drupal install
   */
  public function localizeVideos() {
    $keep_source = variable_get('video_mediamosa_keep_source', 1);
    
    //Check for video's in the queue and process them
    $result = db_query("SELECT
      {files}.fid,
      {files}.uid,
      {files}.filename,
      {files}.filepath,
      {files}.filemime,
      {files}.filesize,
      {files}.status AS file_status,
      {files}.timestamp,
      {video_mediamosa}.*
    FROM {video_mediamosa} LEFT JOIN {files} USING(fid)
    WHERE
      {video_mediamosa}.status = %d",
      VIDEO_MEDIAMOSA_STATUS_ENCODED);
    
    //loop and download
    while ($file = db_fetch_object($result)) {
      $video = $this->download($file);
      //Update video with new thumbnail
      if ($video->status == VIDEO_MEDIAMOSA_STATUS_SUCCESS && $keep_source != 1) {
        file_delete($file->filename);
        db_query('DELETE FROM {files} WHERE fid = %d', $file->fid);
      }
    }
  }
  
  /**
   * Upload a video to the mediamosa platform
   * 
   * @param Stdclass $video (as returned by this->load())
   * @return bool FALSE on failure
   */
  protected function upload($video) {
    $app_id                  = variable_get('video_mediamosa_app_id', '');
    $transcode               = variable_get('video_mediamosa_transcode', '');
    $still_startframe        = variable_get('video_mediamosa_still_startframe', '');
    $still_endframe          = variable_get('video_mediamosa_still_endframe', '');
    $still_every_second      = variable_get('video_mediamosa_still_every_second', '');
    $num_still_per_mediafile = variable_get('video_mediamosa_num_still_per_mediafile', '');
    $still_type              = variable_get('video_mediamosa_still_type', '');
    
    //Check for asset_id
    if (empty($video->asset_id)) {
      //$video->asset_id = $this->createAsset();
      $response = $this->mediamosa_client->request('POST', '/asset/create');
      if ($this->mediamosa_client->check_result($response)) {
        $video->asset_id = (string)$response->items->item->asset_id;
      }
      else {
        return FALSE;
      }
    }
    
    //Check for src mediaFile id.
    if (empty($video->src_mediafile_id)) {
      $response = $this->mediamosa_client->request(
        'POST', 
        '/mediafile/create', 
        array('asset_id' => $video->asset_id)
      );
      if ($this->mediamosa_client->check_result($response)) {
        $video->src_mediafile_id = (string)$response->items->item->mediafile_id;
      }
      else {
        return FALSE;
      }
    }
    
    //Register transcode id's
    if (empty($transcode)) {
      $video->transcode_ids = implode(',', array_filter($transcode));
    }
    
    $this->update($video);
    
    //Request upload ticket
    $response = $this->mediamosa_client->request(
      'POST', 
      '/mediafile/' . $video->src_mediafile_id . '/uploadticket/create'
    );
    if ($this->mediamosa_client->check_result($response)) {
      $uploadTicket = $response->items->item;
    }
    else {
      return FALSE;
    }
    
    $user = $this->mediamosa_client->get_user();
    
    $uri = '/mediafile/' . $video->src_mediafile_id . '/upload' .
           '?upload_ticket=' . $uploadTicket->ticket_id .
           '&filename=' . $video->filename .
           '&user_id=' . $user[1] .
           '&completed_url=' . urlencode('http://www.' . $_SERVER['HTTP_HOST'] . base_path() . 'video_mediamosa/' . $file->fid . '/status_update') .
           '&create_still=TRUE';
    
    if (!empty($app_id)) {
      $uri .= '&app_id[]=' . implode('&app_id[]=', explode(',', $app_id));
    }
    if (!empty($transcode)) {
      $uri .= '&transcode[]=' . implode('&transcode[]=', array_filter($transcode));
    }
  
    if (!empty($still_type)) {
      $uri .= '&still_type=' . $still_type;
    }
    if (!empty($still_startframe)) {
      $uri .= '&start_frame=' . $still_startframe;
    }
    if (!empty($still_endframe)) {
      $uri .= '&end_frame=' . $still_endframe;
    }
    if (!empty($still_every_second)) {
      $uri .= '&still_every_second=' . $still_every_second;
    }
    if (!empty($num_still_per_mediafile)) {
      $uri .= '&still_per_mediafile=' . $num_still_per_mediafile;
    }
    
    $response = $this->mediamosa_client->request(
      'PUT',
      $uri,
      file_get_contents($_SERVER['DOCUMENT_ROOT'] . base_path() . $video->filepath)
    );
    
    if (variable_get('video_mediamosa_download_videos', 1)) {
      $this->mediamosa_client->request(
        'POST', 
        '/mediafile/' . $video->src_mediafile_id . '?user_id=' . $user[1] . '&is_downloadable=TRUE'
      );
    }
    return TRUE;
  }
  
  /**
   * Download a file from the mediamosa platform to the local drupal instance
   * 
   * @param Stdclass $video (as returned by this->load())
   */
  protected function download($video) {
    //First get all mediafiles from this asset.
    $response = $this->mediamosa_client->request('GET', '/asset/' . $video->asset_id . '/mediafile');
    $video_path = variable_get('video_mediamosa_files_path', 'videos');
    if (!empty($video_path)) {
      $video_path = '/' . $video_path;
    }
    $still_path = variable_get('video_mediamosa_files_still_path', 'videos/stills');
    if (!empty($still_path)) {
      $still_path = '/' . $still_path;
    }
    
    if ($response !== FALSE) {
      if (empty($video->mediafiles)) {
        $video->mediafiles = Array();
      }
      foreach ($response->items->children() AS $mediafile) {
        $mediafile_id = (string) $mediafile->mediafile_id;
        
        if ((string) $mediafile->is_original_file == 'FALSE') {
          
          if (empty($video->mediafiles[$mediafile_id])) {
            $video->mediafiles[$mediafile_id] = new stdClass();
          }
          
          //Localize
          $response = $this->mediamosa_client->request(
            'GET',
            '/asset/' . $video->asset_id . '/play?mediafile_id=' . $mediafile_id . '&response=download'
          );
          if ($this->mediamosa_client->check_result($response)) {
            //Download video
            $video->mediafiles[$mediafile_id]->fid = $this->download_remote_file(
              (string)$response->items->item->output,
              $video_path . '/' . $mediafile_id . '.' . $mediafile->file_extension,
              Array(
                'mime_type' => (string) $mediafile->still->mime_type,
                'filesize'  => (string) $mediafile->still->filesize,
              ),
              $video->uid
            );
          }
            
          $video->mediafiles[$mediafile_id]->mediafile_id = $mediafile_id;
          $video->mediafiles[$mediafile_id]->transcode_id = (string) $mediafile->transcode_profile_id;
          $video->mediafiles[$mediafile_id]->meta_data = (array) $mediafile->metadata;
        }
        else {
          $stills = count($mediafile->still) > 0
            ? $mediafile->still 
            : array($mediafile->still);
          
          foreach ($stills AS $index => $still) {
            //Download still
            $fid = $this->download_remote_file(
              (string) $still->still_ticket,
              $still_path . '/' . pathinfo($mediafile->filename, PATHINFO_FILENAME) .
               '-' . $index . '.' . (string) $still->file_extension,
              Array(
                'mime_type' => (string) $still->mime_type,
                'filesize'  => (string) $still->filesize,
              )
            );
            //Default the first still as the main still
            if ($index == 0) {
              $video->thumbnail_fid = $fid;
            }
            $video->thumbnails[] = $fid;
          }
        }
      }
      //Update video and set to success.
      $video->status = VIDEO_MEDIAMOSA_STATUS_SUCCESS;
      $this->update($video);
      
      //Remove the asset from the server
      if (variable_get('video_mediamosa_remove_downloaded_asset', 0)) {
        $response = $this->mediamosa_client->request('POST', '/asset/' . $video->asset_id . '/delete?delete=cascade');
      }
      
      module_invoke_all('video_status_update', 'success', $video);
      return $video;
    }
  }
  
  /**
   * Helper method to download a file and create a local record in the files table.
   * 
   * @param string $source source file/url
   * @param string $target the target path
   * @param array $metadata array('mime_type' => '', 'filesize' => '')
   * @param int $uid the uid for the files record.
   */
  protected function download_remote_file($source, $target, $metadata, $uid = NULL) {
    
    $filename = file_save_data(
      file_get_contents($source), 
      file_directory_path() . $target,
      FILE_EXISTS_RENAME
    );
    
    db_query("INSERT INTO {files} (
              filename, filepath, filemime, filesize, status, uid
            ) VALUES(
              '%s',     '%s',     '%s',     '%s',     %d,     %d
            )",
      substr($filename, strrpos($filename, '/')+1),
      $filename,
      $metadata['mime_type'],
      $metadata['filesize'],
      FILE_STATUS_PERMANENT,
      $uid
    );
    return db_last_insert_id('{files}', 'fid');
  }
  
  public function load($fid) {
    $result = db_fetch_object(db_query("SELECT
        {files}.`fid`,
        {files}.`uid`,
        {files}.`filename`,
        {files}.`filepath`,
        {files}.`filemime`,
        {files}.`filesize`,
        {files}.`status` AS file_status,
        {files}.`timestamp`,
        {video_mediamosa}.`asset_id`,
        {video_mediamosa}.`src_mediafile_id`,
        {video_mediamosa}.`transcode_ids`,
        {video_mediamosa}.`thumbnail_fid`,
        {video_mediamosa}.`thumbnails`,
        {video_mediamosa}.`created`,
        {video_mediamosa}.`updated`,
        {video_mediamosa}.`status`
      FROM {video_mediamosa}
      LEFT JOIN {files} USING(`fid`)
      WHERE
        {files}.`fid` = %d", $fid));
    
    $result->thumbnails = !empty($result->thumbnails)
      ? explode(',', $result->thumbnails)
      : Array();
    
    if (!empty($result)) {
      $sql = "SELECT
            {files}.`fid`,
            {files}.`uid`,
            {files}.`filename`,
            {files}.`filepath`,
            {files}.`filemime`,
            {files}.`filesize`,
            {files}.`status` AS file_status,
            {files}.`timestamp`,
            {video_mediamosa_mediafile}.`asset_id`,
            {video_mediamosa_mediafile}.`mediafile_id`,
            {video_mediamosa_mediafile}.`transcode_id`,
            {video_mediamosa_mediafile}.`meta_data`,
       		{video_mediamosa_mediafile}.`status`
            FROM {video_mediamosa_mediafile}
            LEFT JOIN {files} ON 
              {video_mediamosa_mediafile}.`fid` = {files}.`fid`
            WHERE
              `asset_id` = '%s'";
      $mediafiles = db_query($sql, $result->asset_id);
      
      $result->mediafiles = array();
      while ($mediafile = db_fetch_object($mediafiles)) {
        if (!empty($mediafile->meta_data)) {
          $mediafile->meta_data = unserialize(str_replace(array(
              'O:16:"SimpleXMLElement":0:{}',
              'O:16:"SimpleXMLElement":'
            ),
            Array(
              's:0:"";', 
              'O:8:"stdClass":'
            ),
            $mediafile->meta_data
          ));
        }
        $result->mediafiles[$mediafile->mediafile_id] = $mediafile;
      }
      
      //Set video module status
      switch ($result->status) {
        case VIDEO_MEDIAMOSA_STATUS_SUCCESS:
          $result->video_status = VIDEO_RENDERING_COMPLETE;
          break;
        case VIDEO_MEDIAMOSA_STATUS_FAILED:
          $result->video_status = VIDEO_RENDERING_FAILED;
          break;
        default:
          $result->video_status = VIDEO_RENDERING_ACTIVE;
          break;
      }
      
      return $result;
    }
    return FALSE;
  }
  
  public function insert($video) {
    if (empty($video->thumbnails)) {
      $video->thumbnails = Array();
    }
    
    $sql = "INSERT INTO {video_mediamosa} (
              `fid`, `nid`, `asset_id`, `src_mediafile_id`, `transcode_ids`, 
              `thumbnails`, `created`, `updated`, `status`
            )
            VALUES (
              %d,    %d,    '%s',       '%s',               '%s',
              '%s',         %d,        %d,        %d
            )";
    db_query($sql, $video->fid, $video->nid, $video->asset_id, $video->src_mediafile_id, $video->transcode_ids, 
                   implode(',', $video->thumbnails), time(), 0, $video->status);
    
    //Insert/update mediafiles
    if (!empty($video->mediafiles) && !empty($video->mediafile_id)) {
      foreach ($video->mediafiles AS $mediafile) {
        $sql = "INSERT INTO {video_mediamosa_mediafile} (
                  `fid`, `created`, `updated`, `asset_id`, `mediafile_id`, `transcode_id`, `meta_data`, `status`
                )
                VALUES (
                  %d,    %d,        %d,        '%s',       '%s',           %d,             '%s',		%d
                )";
        db_query($sql, $mediafile->fid, time(), time(), $video->asset_id, $mediafile->mediafile_id, $mediafile->transcode_id, serialize($mediafile->meta_data), $mediafile->status);
      }
    }
  }
  
  /**
   * Update mediamosa video properties.
   * 
   * Mediafiles included in video->mediafiles are also updated.
   * 
   * @param unknown_type $video
   */
  public function update($video) {
    if (empty($video->thumbnails)) {
      $video->thumbnails = Array();
    }
    
    $sql = "UPDATE {video_mediamosa} SET
          `nid` = %d,
          `asset_id` = '%s',
          `src_mediafile_id` = '%s',
          `transcode_ids` = '%s',
          `thumbnail_fid` = '%s',
          `thumbnails` = '%s',
          `updated` = %d,
          `status` = %d
           WHERE
             `fid` = %d";
    db_query($sql, $video->nid, $video->asset_id, $video->src_mediafile_id, $video->transcode_ids, $video->thumbnail_fid, implode(',', $video->thumbnails), time(), $video->status, $video->fid);
    
    //Insert/update mediafiles
    if (!empty($video->mediafiles) && !empty($video->asset_id)) {
      foreach ($video->mediafiles AS $mediafile) {
        
        $sql = "INSERT INTO {video_mediamosa_mediafile} (
              `fid`, `created`, `updated`, `asset_id`, `mediafile_id`, `transcode_id`, `meta_data`, `status`
            )
            VALUES (
              %d,  %d,      %d,      '%s',     '%s',         %d,           '%s',      %d
            ) ON DUPLICATE KEY UPDATE
             `fid` = %d,
             `updated` = %d,
             `asset_id` = '%s',
             `mediafile_id` = '%s',
             `transcode_id` = %d,
             `meta_data` = '%s',
             `status` = %d";
        db_query($sql, $mediafile->fid, time(), time(), $video->asset_id, $mediafile->mediafile_id, $mediafile->transcode_id, serialize($mediafile->meta_data), $mediafile->status, 
                 $mediafile->fid, time(), $video->asset_id, $mediafile->mediafile_id, $mediafile->transcode_id, serialize($mediafile->meta_data), $mediafile->status);
      }
    }
  }
  
  /**
   * Mediamosa video 
   * Mediafiles included in video->mediafiles are also updated.
   * 
   * @param unknown_type $video
   */
  public function delete($video) {
    $video_data   = $this->load($video->fid);
    
    //Collect files that will be deleted locally
    $files        = Array();
    if (!empty($video_data->mediafiles)) {
      foreach ($video_data->mediafiles AS $mediafile) {
        $files[$mediafile->fid] = $mediafile->filepath;
      }
    }
    if (!empty($video_data->thumbnails)) {
      $thumbnails = db_query("SELECT fid, filepath FROM {files} WHERE fid IN ('%s')", implode("','", $video_data->thumbnails));
      while ($thumbnail = db_fetch_object($thumbnails)) {
        $files[$mediafile->fid] = $mediafile->filepath;
      }
    }
    
    //Cleanup database
    db_query("DELETE FROM {video_mediamosa} WHERE fid = %d", $video->fid);
    db_query("DELETE FROM {video_mediamosa_mediafile} WHERE asset_id= '%s'", $video_data->asset_id);
    if (!empty($files)) {
      db_query("DELETE FROM {files} WHERE fid IN ('%s')", implode("','", array_keys($files)));
    }
    
    //Delete files
    foreach ($files AS $fid => $filepath) {
      file_delete($_SERVER['DOCUMENT_ROOT'] . $filepath);
    }
    
    //Delete remote asset
    if (!empty($video_data->asset_id)) {
      $this->mediamosa_client->request('POST', '/asset/' . $video_data->asset_id . '/delete', array(
        'delete'  => 'cascade'
      ));
    }
  }
}
