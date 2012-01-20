#include "OpenCV.h"

class Image: public node::ObjectWrap {
  public:
      cv::Mat mat;

      static Persistent<FunctionTemplate> constructor;
      static void Init(Handle<Object> target);
      static Handle<Value> New(const Arguments &args);

      Image(int width, int height);   
      Image(v8::Value* src);      
      Image(uint8_t* buf, unsigned len);      
            
      static Handle<Value> GetWidth(Local<String> prop, const AccessorInfo &info);
      static Handle<Value> GetHeight(Local<String> prop, const AccessorInfo &info);

      //static Handle<Value> Convert(const v8::Arguments&);
      static Handle<Value> Save(const v8::Arguments&);
      static Handle<Value> Ellipse(const v8::Arguments&);

      static Handle<Value> ToBuffer(const v8::Arguments&);

  private:
      ~Image();
};
