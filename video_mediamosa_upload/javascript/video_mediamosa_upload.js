/**
 * Use drupal behavior to attach form handlers to process
 * video upload
 */
Drupal.behaviors.video_mediamosa_uploadBehavior = function() {
  //Recheck every form, the class prevents double attachment of events
  $('form').each(function() {
    //Create new on unknown form, otherwise only  
    //look for new video upload form elements
    if (typeof video_mediamosa_upload.forms[this.id] == 'undefined') {
      video_mediamosa_upload.forms[this.id] = new video_mediamosa_upload
       .form($(this));
    }
    else {
      video_mediamosa_upload.forms[this.id].checkForNewElements();
    }
  });
};

/**
 * Base namespace for the module.
 */
var video_mediamosa_upload = {
  iframe: null,
  forms: {},
  form: function(form) {
    var self = this;
    this.elements       = {};
    this.queue          = {};
    this.isUploading    = false;
    this.submitOnFinish = false;
    this.domEl          = form;
    this.videoUploadFields = $('.video_mediamosa_upload');

    this.registerElement = function(element) {
      self.elements[element.id] = element;
      
      //Bind direct upload buttons
      $(element).siblings('.form-submit').bind(
        'click', 
        self.handleDirectUpload
      );
      
      //Bind add-more to own handler to prevent 
      //normal submission of files to server. 
      var name      = element.name.match(/files\[(.*)\]/);
      //Custom handler for add more button, default submits 
      //form and that can't be prevented.
      $('input[name=' + name[1].substring(0, name[1].lastIndexOf('_')) + 
       '_add_more]').bind('click', element, self.handleAddMore);
    };
    this.checkForNewElements = function() {
      $('.video_mediamosa_upload', form).each(function() {
        //Not yet registered
        if (typeof self.elements[this.id] == 'undefined') {
          self.registerElement(this);
        }
      });
    };
    
    this.handleAddMore = function(event) {
      var addMoreButton = this;
      
      var data = {
        form_id: $('input[name=form_id]', form).val(),
        form_build_id: $('input[name=form_build_id]', form).val() 
      };
      var num = event.data.id.match(/([0-9]*)-upload/);
      for(var i = 0; i < num[1]; i++) {
        data[name + '[' + i + ']'] = '';
      }
      $.ajax({
        url: Drupal.settings.basePath + 'content/js_add_more/' + 
              $(this).attr('rel'),
        data: data,
        type: 'POST',
        success: function(response) {
          var result = $(Drupal.parseJson(response).data);
          var table = $('#' + $(addMoreButton).attr('rel').split('/')[1] + 
           '_values');
          
          $(document.body).append(result.filter('script'));
          
          table.append($('tr:last', result));
          
          //Hack for removing sortable properties, can this be done better?
          //Just applying sortable again adds multiple handles...
          table.removeClass('tabledrag-processed');
          table.find('.tabledrag-handle').remove();
          table.find('.delta-order').css('display', 'none');
          
          //Attach behaviors to new video field
          Drupal.attachBehaviors($(event.data).parents('table').eq(0));
        }
      });
      return false;
    };
    
    this.handleDirectUpload = function() {
      var element = $(this).siblings('.video_mediamosa_upload');
      if (element.val() != '') {
        form.unbind('submit', self.handleSubmit);
        var item = new video_mediamosa_upload.item(self, element);
        item.onComplete = function() {
          form.bind('submit', self.handleSubmit);
        };
        self.processItem(item, false);
      }
      $(this).hide();
      return false;
    };
    this.handleSubmit = function() {
      //Prevent double submit
      if (form.data('video_mediamosa_upload_active') == true) {
        return false;
      }
      
      //Add fields to queue
      $('.video_mediamosa_upload[value!=""]', form).each(function() {
        var item = new video_mediamosa_upload.item(self, $(this));
        item.onComplete = self.processQueue;
        self.addToQueue(item);
      });
      
      //Disable direct upload buttons
      $('.video_mediamosa_upload_direct').hide();
      
      //Set uploading to active
      form.data('video_mediamosa_upload_active', true);
      
      //Remove this handler and set target to proxy iframe
      form.unbind('submit', self.handleSubmit);

      //Process queue
      self.processQueue();
      
      //Prevent normal submit
      return false;
    };
    this.addToQueue = function(upload_item) {
      this.queue[upload_item.getId()] = upload_item;
    };
    this.processQueue = function() {
      //Get first from array and process it
      for(key in self.queue) {
        self.processItem(self.queue[key], true);
        delete self.queue[key];
        
        return true;
      }
      //Queue is empty.
      
      form.data('video_mediamosa_upload_active', false);

      //Restore action and target
      form.attr('action', form.data('original_action'));
      form.attr('target', form.data('original_target'));
      
      self.revertForm();
      
      form.submit();
      
      return false;
    };
    this.processItem = function(item, continueQueue) {
      $.ajax({
        url: Drupal.settings.basePath + 
              'video_mediamosa_upload/ajax/request_upload?filename=' + 
              item.getElement().val(),
        success: function(sReturnData) {
          var uploadticket = Drupal.parseJson(sReturnData);
          item.upload(uploadticket);
        },
        error: function() { window.alert('Error fetching upload ticket.'); }
      });
    };
    this.prepareForm = function(uploadticket) {
      self.revertForm();
      
      var additional_form_elements = '';
      
      var length = uploadticket.transcodes.length;
      for(var i = 0; i < length; i++){
        additional_form_elements += '<input class="video_mediamosa_upload_temp" type="hidden" name="transcode[]" value="' + uploadticket.transcodes[i] + '" />';
      }
      if (typeof uploadticket.app_id != 'undefined' && uploadticket.app_id != '') {
        additional_form_elements += '<input class="video_mediamosa_upload_temp" type="hidden" name="app_id[]" value="' + uploadticket.app_id + '" />';
      }
      if (typeof uploadticket.still_type != 'undefined' && uploadticket.still_type != '') {
        additional_form_elements += '<input class="video_mediamosa_upload_temp" type="hidden" name="still_type" value="' +
                      uploadticket.still_type + '" />';
                      
        if (typeof uploadticket.still_per_mediafile != 'undefined' && uploadticket.still_per_mediafile != '') {
          additional_form_elements += '<input class="video_mediamosa_upload_temp" type="hidden" name="still_per_mediafile" value="' +
                        uploadticket.still_per_mediafile + '" />';
        }
        if (typeof uploadticket.still_startframe != 'undefined' && uploadticket.still_startframe != '') {
          additional_form_elements += '<input class="video_mediamosa_upload_temp" type="hidden" name="start_frame" value="' +
                        uploadticket.still_startframe + '" />';
        }
        if (typeof uploadticket.still_endframe != 'undefined' && uploadticket.still_endframe > 0) {
          additional_form_elements += '<input class="video_mediamosa_upload_temp" type="hidden" name="end_frame" value="' +
                        uploadticket.still_endframe + '" />';
        }
        if (typeof uploadticket.still_every_second != 'undefined' && uploadticket.still_every_second != '') {
          additional_form_elements += '<input class="video_mediamosa_upload_temp" type="hidden" name="still_every_second" value="' +
                        uploadticket.still_every_second + '" />';
        }
      }
      
      additional_form_elements += '<input class="video_mediamosa_upload_temp" type="hidden" name="APC_UPLOAD_PROGRESS" value="' + uploadticket.upload_progress_id +'" />';
      
      additional_form_elements += '<input class="video_mediamosa_upload_temp" type="hidden" name="create_still" value="true" />';
      additional_form_elements += '<input class="video_mediamosa_upload_temp" type="hidden" name="completed_transcoding_url" value="' + uploadticket.completed_url + '" />';

      form.prepend(additional_form_elements);
    };
    this.revertForm = function(item_id) {
      $('.video_mediamosa_upload_temp', form).remove();
    };
    
    //**** Constructor ****
    
    //Add proxy iframe to dom if not available.
    if (video_mediamosa_upload.iframe == null) {
      $(document.body).append('<iframe ' + 
        ' style="position: absolute; display: none;"' + 
        ' height="0"' +
        ' width="0"' +
        ' name="video_mediamosa_upload_proxy"' +
        ' id="video_mediamosa_upload_proxy"></iframe>');
      video_mediamosa_upload.iframe = $('#video_mediamosa_upload_proxy');
    }

    form.data('original_action', form.attr('action'));
    form.data('original_target', form.attr('target'));
    
    form.bind('submit', this.handleSubmit);

    self.checkForNewElements();
  },
  item: function(form, element) {
    var self = this;
    
    this.getId = function() {
      return element.attr('id');
    },
    this.getForm = function() {
      return form;
    },
    this.getElement = function() {
      return element;
    },
    this.upload = function(uploadticket) {
      if (uploadticket != null && typeof uploadticket.action != 'undefined') {
        uploadticket.upload_progress_id = uploadticket.mediafile_id + 'Apc';
        form.prepareForm(uploadticket);
        
        //Set name of upload field to MediaMosa required name.
        element.data('original_name', element.attr('name'));
        element.attr('name', 'file');

        //Init form dom element
        form.domEl.attr('action', uploadticket.action);
        form.domEl.attr('target', 'video_mediamosa_upload_proxy');
        
        //Update fid in form element to point to 
        //the temp file from the uploadticket
        var name      = element.data('original_name').match(/files\[(.*)\]/);
        var hidden_id = 'edit-' + name[1].substring(0, name[1]
                          .lastIndexOf('_')).replace(/_/g, '-') +
                          '-' + name[1].substring(name[1].lastIndexOf('_') + 1)+
                          '-fid';
        $('#' + hidden_id).val(uploadticket.fid);
        
        //Upload progress indicator
        self.progress(uploadticket, null, null, true);
        
        //Upload complete
        video_mediamosa_upload.iframe.bind('load', self.complete);
        
        //Submit form to iframe
        form.domEl.submit();
      }
    };
    this.complete = function() {
      //Unbind iframe load listener
      video_mediamosa_upload.iframe.unbind('load', self.complete);
      
      //Find fid field
    	element.remove();
      if (typeof self.onComplete != 'undefined') {
        self.onComplete();
      }
    };
    this.progress = function(uploadticket, bar, text, init) {
      if (typeof init != 'undefined' && init == true) {
        element.before('<div class="video_mediamosa_upload_progress">' +
            '<div class="progress_bar"></div>' + 
            '<div class="progress_text">0%</div>' + 
          '</div>');
        
        var upload_progress_container = 
         element.siblings('.video_mediamosa_upload_progress');
        
        upload_progress_container.width(element.width());
        
        bar = upload_progress_container.find('.progress_bar');
        text = upload_progress_container.find('.progress_text');

        element.hide();
      }
      
      $.ajax({
        url : Drupal.settings.basePath + 
               'video_mediamosa_upload/ajax/request_uploadprogress',
        data: {
          url      : uploadticket.uploadprogress_url + '?id=' + 
                      uploadticket.upload_progress_id
        },
        type: 'GET',
        success: function(response) {
          var progress = Drupal.parseJson(response);
          if (progress.percentage < 0) {
            progress.percentage = 0;
          }
          text.html(progress.percentage + '%');
          bar.width(progress.percentage + '%');
          
          //Stop polling when finished..
          if (progress.percentage < 100) {
            self.progress(uploadticket, bar, text);
          }
        }
      });
    };
  }
};