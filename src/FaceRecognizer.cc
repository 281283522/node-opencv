#include "FaceRecognizer.h"
#include "OpenCV.h"

#if CV_MAJOR_VERSION >= 2 && CV_MINOR_VERSION >=4

#include "Matrix.h"


// Todo, move somewhere useful
cv::Mat fromMatrixOrFilename(Local<Value> v){
  cv::Mat im;
  if (v->IsString()){
    std::string filename = std::string(*v8::String::AsciiValue(v->ToString()));
    im = cv::imread(filename);
    //std::cout<< im.size();
  } else {
    Matrix *img = ObjectWrap::Unwrap<Matrix>(v->ToObject());
    im = img->mat;
  }
  return im;
}


void AsyncPredict(uv_work_t *req);
void AfterAsyncPredict(uv_work_t *req);

Persistent<FunctionTemplate> FaceRecognizerWrap::constructor;

void
FaceRecognizerWrap::Init(Handle<Object> target) {
    HandleScope scope;

    // Constructor
      constructor = Persistent<FunctionTemplate>::New(FunctionTemplate::New(FaceRecognizerWrap::New));
      constructor->InstanceTemplate()->SetInternalFieldCount(1);
      constructor->SetClassName(String::NewSymbol("FaceRecognizer"));

    NODE_SET_METHOD(constructor, "createLBPHFaceRecognizer", CreateLBPH);
    NODE_SET_METHOD(constructor, "createEigenFaceRecognizer", CreateEigen);
    NODE_SET_METHOD(constructor, "createFisherFaceRecognizer", CreateFisher);

    NODE_SET_PROTOTYPE_METHOD(constructor, "train", Train);
    NODE_SET_PROTOTYPE_METHOD(constructor, "predictSync", PredictSync);


    target->Set(String::NewSymbol("FaceRecognizer"), constructor->GetFunction());
};

Handle<Value>
FaceRecognizerWrap::New(const Arguments &args) {
  HandleScope scope;

  if (args.This()->InternalFieldCount() == 0)
    JSTHROW_TYPE("Cannot Instantiate without new")

  // By default initialize LBPH
  cv::Ptr<cv::FaceRecognizer> f = cv::createLBPHFaceRecognizer(1, 8, 8, 8, 80.0);
  FaceRecognizerWrap *pt = new FaceRecognizerWrap(f);

  pt->Wrap(args.This());
  return args.This();
}

Handle<Value>
FaceRecognizerWrap::CreateLBPH(const Arguments &args) {
  HandleScope scope;

  int radius = 1;
  int neighbors = 8;
  int grid_x = 8;
  int grid_y = 8;
  double threshold = 80;

  INT_FROM_ARGS(radius, 0)
  INT_FROM_ARGS(neighbors, 1)
  INT_FROM_ARGS(grid_x, 2)
  INT_FROM_ARGS(grid_y, 3)
  DOUBLE_FROM_ARGS(threshold, 4)

  Local<Object> n = FaceRecognizerWrap::constructor->GetFunction()->NewInstance();

  cv::Ptr<cv::FaceRecognizer> f = cv::createLBPHFaceRecognizer(
      radius, neighbors, grid_x, grid_y, threshold
  );
  FaceRecognizerWrap *pt = new FaceRecognizerWrap(f);

  pt->Wrap(n);
  return n;
}

Handle<Value>
FaceRecognizerWrap::CreateEigen(const Arguments &args) {
  HandleScope scope;

  int components = 0;
  double threshold = DBL_MAX;

  INT_FROM_ARGS(components, 0)
  DOUBLE_FROM_ARGS(threshold, 1)

  Local<Object> n = FaceRecognizerWrap::constructor->GetFunction()->NewInstance();

  cv::Ptr<cv::FaceRecognizer> f = cv::createEigenFaceRecognizer(
      components, threshold
  );
  FaceRecognizerWrap *pt = new FaceRecognizerWrap(f);

  pt->Wrap(n);
  return n;
}

Handle<Value>
FaceRecognizerWrap::CreateFisher(const Arguments &args) {
  HandleScope scope;

  int components = 0;
  double threshold = DBL_MAX;

  INT_FROM_ARGS(components, 0)
  DOUBLE_FROM_ARGS(threshold, 1)

  Local<Object> n = FaceRecognizerWrap::constructor->GetFunction()->NewInstance();

  cv::Ptr<cv::FaceRecognizer> f = cv::createFisherFaceRecognizer(
      components, threshold
  );
  FaceRecognizerWrap *pt = new FaceRecognizerWrap(f);

  pt->Wrap(n);
  return n;
}


FaceRecognizerWrap::FaceRecognizerWrap(cv::Ptr<cv::FaceRecognizer> f){
  rec = f; 
}

Handle<Value>
FaceRecognizerWrap::Train(const Arguments& args){
	SETUP_FUNCTION(FaceRecognizerWrap)

  cv::vector<cv::Mat> images;
  cv::vector<int> labels;

  if (args.Length() < 1 || !args[0]->IsArray()){
    JSTHROW("FaceRecognizer.train takes a list of [<int> label, image] tuples")
  }
 
  // Iterate through [[label, image], ...] etc, and add matrix / label to vectors
  const Local<Array> tuples = v8::Array::Cast(*args[0]);
  const uint32_t length = tuples->Length();
  for (uint32_t i=0 ; i<length ; ++i){
      const Local<Value> val = tuples->Get(i);

      if (!val->IsArray()){
        JSTHROW("train takes a list of [label, image] tuples")
      }

      Local<Array> valarr = v8::Array::Cast(*val);

      if (valarr->Length() != 2 || !valarr->Get(0)->IsInt32()){
        JSTHROW("train takes a list of [label, image] tuples")
      }

	   int label = valarr->Get(0)->Uint32Value();
     cv::Mat im = fromMatrixOrFilename(valarr->Get(1));
     im = im.clone();
     cv::cvtColor(im, im, CV_RGB2GRAY);
     labels.push_back(label);
     images.push_back(im);
  }

  self->rec->train(images, labels);

  return scope.Close(v8::Undefined());
}


Handle<Value>
FaceRecognizerWrap::PredictSync(const Arguments& args){
	SETUP_FUNCTION(FaceRecognizerWrap)

  cv::Mat im = fromMatrixOrFilename(args[0]);//TODO CHECK!
  cv::cvtColor(im, im, CV_RGB2GRAY);
 // int predictedLabel = self->rec->predict(im);

  int predictedLabel = -1;
  double confidence = 0.0;
  self->rec->predict(im, predictedLabel, confidence);

  v8::Local<v8::Object> res = v8::Object::New();
  res->Set(v8::String::New("id"), v8::Number::New(predictedLabel));
  res->Set(v8::String::New("confidence"), v8::Number::New(confidence));

  return scope.Close(res);
}
#endif // End version > 2.4
