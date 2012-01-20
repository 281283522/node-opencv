#include "VideoCaptureWrap.h"
#include "Image.h"
#include "OpenCV.h"

v8::Persistent<FunctionTemplate> VideoCaptureWrap::constructor;

void
VideoCaptureWrap::Init(Handle<Object> target) {
    HandleScope scope;

    // Constructor
    constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(VideoCaptureWrap::New));
    constructor->InstanceTemplate()->SetInternalFieldCount(1);
    constructor->SetClassName(String::NewSymbol("VideoCapture"));

    // Prototype
    Local<ObjectTemplate> proto = constructor->PrototypeTemplate();
    
    NODE_SET_PROTOTYPE_METHOD(constructor, "getFrame", GetFrame);

    target->Set(String::NewSymbol("VideoCapture"), constructor->GetFunction());
};    

Handle<Value>
VideoCaptureWrap::New(const Arguments &args) {
  HandleScope scope;

  if (args.This()->InternalFieldCount() == 0)
    return v8::ThrowException(v8::Exception::TypeError(v8::String::New("Cannot Instantiate without new")));

  VideoCaptureWrap *v;

  if (args[0]->IsNumber()){
    v = new VideoCaptureWrap(args[0]->NumberValue());
  } else {}  
  v->Wrap(args.This());
  return args.This();
}


VideoCaptureWrap::VideoCaptureWrap(int device){
  HandleScope scope;

  cv::VideoCapture cap(device);

  if(!cap.isOpened()){
  	ThrowException(Exception::Error(String::New("Camera could not be opened")));
  }
}


Handle<Value>
VideoCaptureWrap::GetFrame(const Arguments &args) {
	HandleScope scope;

	VideoCaptureWrap *v = ObjectWrap::Unwrap<VideoCaptureWrap>(args.This());

	cv::Mat frame;
    v->cap.retrieve(frame);



    Local<Object> im_h = Image::constructor->GetFunction()->NewInstance();
    Image *im = ObjectWrap::Unwrap<Image>(im_h);
    im->mat = frame;
    return scope.Close(im_h);
}

  
