<?php
// $Id$
/**
 * @file
 * synetic_video widget hooks and callbacks.
 */

/**
 * The ajax callback to request upload information
 * 
 * Sends the information back as a json string 
 */
function video_mediamosa_upload_ajax_request_upload() {
  global $user;
  $mediamosa_client = mediamosa_client_get_client();
  $transcode = array_filter(variable_get('video_mediamosa_transcode', ''));
  
  //Create an asset
  $response = $mediamosa_client->request('POST', '/asset/create');
  $asset_id = $mediamosa_client->check_result($response)
   ? (string)$response->items->item->asset_id
   : FALSE;
  //Creating an asset failed, report error to the client.
  if ($asset_id === FALSE) {
    echo "error";
    exit;
  }
   //Create a mediafile to which the file will be uploaded
  $response = $mediamosa_client->request(
    'POST', 
    '/mediafile/create', 
    array('asset_id' => $asset_id)
  );
  $mediafile_id = $mediamosa_client->check_result($response)
   ? (string)$response->items->item->mediafile_id
   : FALSE;
  //Creating a mediafile failed, report error to the client.
  if ($mediafile_id === FALSE) {
    echo "error";
    exit;
  }
  
   //Create an uploadticket for the mediafile
  $response = $mediamosa_client->request(
    'POST', 
    '/mediafile/' . $mediafile_id . '/uploadticket/create'
  );
  $ticket = $mediamosa_client->check_result($response)
    ? $response->items->item
    : FALSE;
  //Creating an upload ticket failed, report error to the client.
  if ($ticket === FALSE) {
    echo "error";
    exit;
  }
  
  //Insert temp file into drupal database, placeholder  
  //for the video_mediamosa transcoder
  $file = new stdClass();
  $file->uid       = $user->uid;
  $file->status    = FILE_STATUS_TEMPORARY;
  $file->timestamp = time();
  $file->filename  = !empty($_GET['filename']) ? $_GET['filename'] : '';
  drupal_write_record('files', $file);
  
  //Add file to the video_mediamosa transcoder queue, with the status uploading
  //it is assumed that after requesting the upload tickey 
  $file->asset_id         = $asset_id;
  $file->src_mediafile_id = $mediafile_id;
  $file->status           = VIDEO_MEDIAMOSA_STATUS_UPLOADING;
  $file->transcode_ids    = implode(',', $transcode);
  
  $video_mediamosa = new video_mediamosa();
  $video_mediamosa->insert($file);
  
  //Collect uploadticket information
  $uploadticket = new stdClass();
  $uploadticket->asset_id            = $asset_id;
  $uploadticket->mediafile_id        = $mediafile_id;
  $uploadticket->ticket_id           = (string) $ticket->ticket_id;
  $uploadticket->fid                 = $file->fid;
  $uploadticket->error               = 'none';
  $uploadticket->action              = (string) $ticket->action;
  $uploadticket->completed_url       = 'http://' . $_SERVER['HTTP_HOST'] . base_path() . 'video_mediamosa/' . $file->fid . '/status_update';
  $uploadticket->uploadprogress_url  = (string) $ticket->uploadprogress_url;
  $uploadticket->transcodes          = array_values(array_filter(variable_get('video_mediamosa_transcode', '')));
  $uploadticket->app_id              = variable_get('video_mediamosa_app_id', '');
  $uploadticket->create_still        = 'true';
  $uploadticket->still_startframe    = (int)variable_get('video_mediamosa_still_startframe', '');
  $uploadticket->still_endframe      = (int)variable_get('video_mediamosa_still_endframe', '');
  $uploadticket->still_every_second  = (int)variable_get('video_mediamosa_still_every_second', '');
  $uploadticket->still_per_mediafile = (int)variable_get('video_mediamosa_num_still_per_mediafile', '');
  $uploadticket->still_type          = variable_get('video_mediamosa_still_type', '');
  
  drupal_json($uploadticket);
}

/**
 * Proxy for requesting the upload progress. Cannot be requested directly from
 * javascript because of cross-site scripting limitations.
 */
function video_mediamosa_upload_ajax_request_upload_progress() {
  $mediamosa_client = mediamosa_client_get_client();
  $result = $mediamosa_client->request('GET', $_GET['url'], NULL, FALSE);
  echo $result;
}

/**
 * Create an uploadticket for mediamosa and return all 
 * required information for uploading a video.
 */
function video_mediamosa_upload_create_ticket() {
  return $uploadticket;
}
