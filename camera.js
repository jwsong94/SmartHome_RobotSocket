var express = require('express'),
      http = require('http'),
      app = express(),
      server = http.createServer(app) ;
app.use(express.static(__dirname + '/images')) ;
var img_flag = 0 ;

var cameraOptions = {
      width : 200,
      height : 200,
      mode : 'timelapse',
      awb : 'off',
      encoding : 'jpg',
      output : 'images/camera.jpg',
      q : 50,
      timeout : 100,
      timelapse : 0,
      nopreview : true,
      th : '0:0:0'
};

var camera = new require('raspicam')(cameraOptions) ;
camera.start() ;

camera.on('exit', function() {
            camera.stop() ;
            console.log('Restart camera') ;
            camera.start() ;
         }) ;

camera.on('read', function() {
            img_flag = 1 ;
              }) ;

app.get('/cam', function(req, res) {
            res.sendfile('camera.html', {root : __dirname}) ;
              }) ;

app.get('/img', function (req, res) {
            console.log('get /img') ;
            if (img_flag == 1) {
                 img_flag = 0 ;
                 res.sendfile('images/camera.jpg') ;
            }
        }) ;
server.listen(7500, function() {
            console.log('express server listening on port ' + server.address().port) ;
              }) ;
