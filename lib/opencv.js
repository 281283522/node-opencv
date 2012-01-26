var Stream = require('stream').Stream
  , Buffers = require('buffers')
  , util = require('util');

var bindings = require('./bindings')

var cv = module.exports = {};

cv.__proto__ = bindings;
/*

# Matrix #
The matrix is one of opencv's most core datatypes.

*/


var matrix = cv.Matrix.prototype;

matrix.faceDetect = function(classifier, opts, cb){
	var face_cascade = new cv.CascadeClassifier("./examples/haarcascade_frontalface_alt.xml");
	face_cascade.detectMultiScale(this, cb);
}

matrix.inspect = function(){
	var size = this.size() ? (this.size()[0] + 'x' + this.size()[1]) : '';

	return "[Matrix " + size + " ]";

}

cv.ImageStream = function(){
	this.data = Buffers([])
	this.writable = true
}

util.inherits(cv.ImageStream, Stream);
var imagestream = cv.ImageStream.prototype;

imagestream.write = function(buf){
	this.data.push(buf)	
	return true;
}


imagestream.end = function(b){
	if (b)
	  imagestream.write.call(this,b);

	var buf = this.data.toBuffer();

	var im = cv.readImage(buf);
	this.emit('load', im);
}

