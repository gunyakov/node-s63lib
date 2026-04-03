#include <napi.h>

#include <memory>
#include <mutex>
#include <string>
#include <utility>

#include "s63lib/s63/s63.h"
#include "s63lib/s63/s63client.h"

namespace {

std::mutex g_s63Mutex;

Napi::Value ThrowTypeError(Napi::Env env, const std::string& message) {
    Napi::TypeError::New(env, message).ThrowAsJavaScriptException();
    return env.Null();
}

bool IsBufferOrString(const Napi::Value& value) {
    return value.IsBuffer() || value.IsString();
}

std::string ToBinaryString(const Napi::Value& value) {
    if (value.IsBuffer()) {
        Napi::Buffer<char> buffer = value.As<Napi::Buffer<char>>();
        return std::string(buffer.Data(), buffer.Length());
    }
    return value.As<Napi::String>().Utf8Value();
}

Napi::Buffer<char> ToBuffer(Napi::Env env, const std::string& value) {
    return Napi::Buffer<char>::Copy(env, value.data(), value.size());
}

Napi::Object BuildDecryptResult(Napi::Env env, S63Error error, const std::string& data) {
    Napi::Object result = Napi::Object::New(env);
    result.Set("error", Napi::Number::New(env, static_cast<int>(error)));
    if (error == S63_ERR_OK) {
        result.Set("data", ToBuffer(env, data));
    } else {
        result.Set("data", env.Null());
    }
    return result;
}

Napi::Value ValidateCellPermitWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() != 2 || !info[0].IsString() || !info[1].IsString()) {
        return ThrowTypeError(env, "validateCellPermit(permit, hwId) expects 2 string arguments");
    }

    const std::string permit = info[0].As<Napi::String>().Utf8Value();
    const std::string hwId = info[1].As<Napi::String>().Utf8Value();
    std::lock_guard<std::mutex> lock(g_s63Mutex);
    return Napi::Boolean::New(env, S63::validateCellPermit(permit, hwId));
}

Napi::Value CreateUserPermitWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() != 3 || !info[0].IsString() || !info[1].IsString() || !info[2].IsString()) {
        return ThrowTypeError(env, "createUserPermit(mKey, hwId, mId) expects 3 string arguments");
    }

    const std::string mKey = info[0].As<Napi::String>().Utf8Value();
    const std::string hwId = info[1].As<Napi::String>().Utf8Value();
    const std::string mId = info[2].As<Napi::String>().Utf8Value();
    std::lock_guard<std::mutex> lock(g_s63Mutex);
    return Napi::String::New(env, S63::createUserPermit(mKey, hwId, mId));
}

Napi::Value ExtractHwIdFromUserPermitWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() != 2 || !info[0].IsString() || !info[1].IsString()) {
        return ThrowTypeError(env, "extractHwIdFromUserpermit(userPermit, mKey) expects 2 string arguments");
    }

    const std::string userPermit = info[0].As<Napi::String>().Utf8Value();
    const std::string mKey = info[1].As<Napi::String>().Utf8Value();
    std::lock_guard<std::mutex> lock(g_s63Mutex);
    return Napi::String::New(env, S63::extractHwIdFromUserpermit(userPermit, mKey));
}

Napi::Value CreateCellPermitWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() != 5 || !info[0].IsString() || !IsBufferOrString(info[1]) ||
            !IsBufferOrString(info[2]) || !info[3].IsString() || !info[4].IsString()) {
        return ThrowTypeError(env, "createCellPermit(hwId, ck1, ck2, cellName, expiryDate) expects (string, Buffer|string, Buffer|string, string, string)");
    }

    const std::string hwId = info[0].As<Napi::String>().Utf8Value();
    const std::string ck1 = ToBinaryString(info[1]);
    const std::string ck2 = ToBinaryString(info[2]);
    const std::string cellName = info[3].As<Napi::String>().Utf8Value();
    const std::string expiryDate = info[4].As<Napi::String>().Utf8Value();

    std::lock_guard<std::mutex> lock(g_s63Mutex);
    return Napi::String::New(env, S63::createCellPermit(hwId, ck1, ck2, cellName, expiryDate));
}

Napi::Value ExtractCellKeysFromCellPermitWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() != 2 || !info[0].IsString() || !info[1].IsString()) {
        return ThrowTypeError(env, "extractCellKeysFromCellpermit(cellPermit, hwId) expects 2 string arguments");
    }

    const std::string cellPermit = info[0].As<Napi::String>().Utf8Value();
    const std::string hwId = info[1].As<Napi::String>().Utf8Value();
    bool ok = false;
    std::pair<std::string, std::string> keys;
    {
        std::lock_guard<std::mutex> lock(g_s63Mutex);
        keys = S63::extractCellKeysFromCellpermit(cellPermit, hwId, ok);
    }

    Napi::Object result = Napi::Object::New(env);
    result.Set("ok", Napi::Boolean::New(env, ok));
    result.Set("key1", ToBuffer(env, keys.first));
    result.Set("key2", ToBuffer(env, keys.second));
    return result;
}

Napi::Value DecryptCellFromPathWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() != 3 || !info[0].IsString() || !IsBufferOrString(info[1]) || !IsBufferOrString(info[2])) {
        return ThrowTypeError(env, "decryptCellFromPath(path, key1, key2) expects (string, Buffer|string, Buffer|string)");
    }

    const std::string path = info[0].As<Napi::String>().Utf8Value();
    const std::string key1 = ToBinaryString(info[1]);
    const std::string key2 = ToBinaryString(info[2]);
    std::string out;
    S63Error error;
    {
        std::lock_guard<std::mutex> lock(g_s63Mutex);
        error = S63::decryptCell(path, {key1, key2}, out);
    }
    return BuildDecryptResult(env, error, out);
}

Napi::Value DecryptCellBufferWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() != 2 || !IsBufferOrString(info[0]) || !IsBufferOrString(info[1])) {
        return ThrowTypeError(env, "decryptCellBuffer(data, key) expects (Buffer|string, Buffer|string)");
    }

    std::string data = ToBinaryString(info[0]);
    const std::string key = ToBinaryString(info[1]);
    S63Error error;
    {
        std::lock_guard<std::mutex> lock(g_s63Mutex);
        error = S63::decryptCell(data, key);
    }
    return BuildDecryptResult(env, error, data);
}

Napi::Value EncryptCellBufferWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() != 2 || !IsBufferOrString(info[0]) || !IsBufferOrString(info[1])) {
        return ThrowTypeError(env, "encryptCellBuffer(data, key) expects (Buffer|string, Buffer|string)");
    }

    std::string data = ToBinaryString(info[0]);
    const std::string key = ToBinaryString(info[1]);
    {
        std::lock_guard<std::mutex> lock(g_s63Mutex);
        S63::encryptCell(data, key);
    }
    return ToBuffer(env, data);
}

Napi::Value DecryptAndUnzipCellByKeyWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() != 4 || !info[0].IsString() || !IsBufferOrString(info[1]) ||
            !IsBufferOrString(info[2]) || !info[3].IsString()) {
        return ThrowTypeError(env, "decryptAndUnzipCellByKey(inPath, key1, key2, outPath) expects (string, Buffer|string, Buffer|string, string)");
    }

    const std::string inPath = info[0].As<Napi::String>().Utf8Value();
    const std::string key1 = ToBinaryString(info[1]);
    const std::string key2 = ToBinaryString(info[2]);
    const std::string outPath = info[3].As<Napi::String>().Utf8Value();
    S63Error error;
    {
        std::lock_guard<std::mutex> lock(g_s63Mutex);
        error = S63::decryptAndUnzipCellByKey(inPath, {key1, key2}, outPath);
    }
    return Napi::Number::New(env, static_cast<int>(error));
}

class DecryptCellFromPathWorker : public Napi::AsyncWorker {
 public:
    DecryptCellFromPathWorker(Napi::Env env, std::string path, std::string key1, std::string key2)
        : Napi::AsyncWorker(env),
          deferred_(Napi::Promise::Deferred::New(env)),
          path_(std::move(path)),
          key1_(std::move(key1)),
          key2_(std::move(key2)),
          error_(S63_ERR_DATA) {}

    Napi::Promise GetPromise() { return deferred_.Promise(); }

    void Execute() override {
        std::lock_guard<std::mutex> lock(g_s63Mutex);
        error_ = S63::decryptCell(path_, {key1_, key2_}, out_);
    }

    void OnOK() override {
        deferred_.Resolve(BuildDecryptResult(Env(), error_, out_));
    }

    void OnError(const Napi::Error& error) override {
        deferred_.Reject(error.Value());
    }

 private:
    Napi::Promise::Deferred deferred_;
    std::string path_;
    std::string key1_;
    std::string key2_;
    std::string out_;
    S63Error error_;
};

class DecryptCellBufferWorker : public Napi::AsyncWorker {
 public:
    DecryptCellBufferWorker(Napi::Env env, std::string data, std::string key)
        : Napi::AsyncWorker(env),
          deferred_(Napi::Promise::Deferred::New(env)),
          data_(std::move(data)),
          key_(std::move(key)),
          error_(S63_ERR_DATA) {}

    Napi::Promise GetPromise() { return deferred_.Promise(); }

    void Execute() override {
        std::lock_guard<std::mutex> lock(g_s63Mutex);
        error_ = S63::decryptCell(data_, key_);
    }

    void OnOK() override {
        deferred_.Resolve(BuildDecryptResult(Env(), error_, data_));
    }

    void OnError(const Napi::Error& error) override {
        deferred_.Reject(error.Value());
    }

 private:
    Napi::Promise::Deferred deferred_;
    std::string data_;
    std::string key_;
    S63Error error_;
};

class DecryptAndUnzipCellByKeyWorker : public Napi::AsyncWorker {
 public:
    DecryptAndUnzipCellByKeyWorker(Napi::Env env, std::string inPath, std::string key1, std::string key2,
                                   std::string outPath)
        : Napi::AsyncWorker(env),
          deferred_(Napi::Promise::Deferred::New(env)),
          inPath_(std::move(inPath)),
          key1_(std::move(key1)),
          key2_(std::move(key2)),
          outPath_(std::move(outPath)),
          error_(S63_ERR_DATA) {}

    Napi::Promise GetPromise() { return deferred_.Promise(); }

    void Execute() override {
        std::lock_guard<std::mutex> lock(g_s63Mutex);
        error_ = S63::decryptAndUnzipCellByKey(inPath_, {key1_, key2_}, outPath_);
    }

    void OnOK() override {
        deferred_.Resolve(Napi::Number::New(Env(), static_cast<int>(error_)));
    }

    void OnError(const Napi::Error& error) override {
        deferred_.Reject(error.Value());
    }

 private:
    Napi::Promise::Deferred deferred_;
    std::string inPath_;
    std::string key1_;
    std::string key2_;
    std::string outPath_;
    S63Error error_;
};

Napi::Value DecryptCellFromPathAsyncWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() != 3 || !info[0].IsString() || !IsBufferOrString(info[1]) || !IsBufferOrString(info[2])) {
        return ThrowTypeError(env, "decryptCellFromPathAsync(path, key1, key2) expects (string, Buffer|string, Buffer|string)");
    }

    auto* worker = new DecryptCellFromPathWorker(env, info[0].As<Napi::String>().Utf8Value(),
                                                  ToBinaryString(info[1]), ToBinaryString(info[2]));
    Napi::Promise promise = worker->GetPromise();
    worker->Queue();
    return promise;
}

Napi::Value DecryptCellBufferAsyncWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() != 2 || !IsBufferOrString(info[0]) || !IsBufferOrString(info[1])) {
        return ThrowTypeError(env, "decryptCellBufferAsync(data, key) expects (Buffer|string, Buffer|string)");
    }

    auto* worker = new DecryptCellBufferWorker(env, ToBinaryString(info[0]), ToBinaryString(info[1]));
    Napi::Promise promise = worker->GetPromise();
    worker->Queue();
    return promise;
}

Napi::Value DecryptAndUnzipCellByKeyAsyncWrapped(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() != 4 || !info[0].IsString() || !IsBufferOrString(info[1]) ||
            !IsBufferOrString(info[2]) || !info[3].IsString()) {
        return ThrowTypeError(env, "decryptAndUnzipCellByKeyAsync(inPath, key1, key2, outPath) expects (string, Buffer|string, Buffer|string, string)");
    }

    auto* worker = new DecryptAndUnzipCellByKeyWorker(env, info[0].As<Napi::String>().Utf8Value(),
                                                       ToBinaryString(info[1]), ToBinaryString(info[2]),
                                                       info[3].As<Napi::String>().Utf8Value());
    Napi::Promise promise = worker->GetPromise();
    worker->Queue();
    return promise;
}

class S63ClientWrap : public Napi::ObjectWrap<S63ClientWrap> {
 public:
    static Napi::Function Init(Napi::Env env) {
        Napi::Function ctor = DefineClass(
                env,
                "S63Client",
                {
                        InstanceMethod("getHWID", &S63ClientWrap::GetHWID),
                        InstanceMethod("getMID", &S63ClientWrap::GetMID),
                        InstanceMethod("getMKEY", &S63ClientWrap::GetMKEY),
                        InstanceMethod("setHWID", &S63ClientWrap::SetHWID),
                        InstanceMethod("setMID", &S63ClientWrap::SetMID),
                        InstanceMethod("setMKEY", &S63ClientWrap::SetMKEY),
                        InstanceMethod("installCellPermit", &S63ClientWrap::InstallCellPermit),
                        InstanceMethod("importPermitFile", &S63ClientWrap::ImportPermitFile),
                        InstanceMethod("getUserpermit", &S63ClientWrap::GetUserpermit),
                        InstanceMethod("open", &S63ClientWrap::Open),
                        InstanceMethod("openAsync", &S63ClientWrap::OpenAsync),
                        InstanceMethod("decryptAndUnzipCell", &S63ClientWrap::DecryptAndUnzipCell),
                        InstanceMethod("decryptAndUnzipCellAsync", &S63ClientWrap::DecryptAndUnzipCellAsync),
                        InstanceMethod("decryptAndUnzipCellWithPermit", &S63ClientWrap::DecryptAndUnzipCellWithPermit),
                        InstanceMethod("decryptAndUnzipCellWithPermitAsync", &S63ClientWrap::DecryptAndUnzipCellWithPermitAsync),
                        InstanceMethod("importPermitFileAsync", &S63ClientWrap::ImportPermitFileAsync),
                });
        return ctor;
    }

    explicit S63ClientWrap(const Napi::CallbackInfo& info) : Napi::ObjectWrap<S63ClientWrap>(info) {
        Napi::Env env = info.Env();
        if (info.Length() != 3 || !info[0].IsString() || !info[1].IsString() || !info[2].IsString()) {
            Napi::TypeError::New(env, "new S63Client(hwId, mKey, mId) expects 3 string arguments")
                    .ThrowAsJavaScriptException();
            return;
        }

        const std::string hwId = info[0].As<Napi::String>().Utf8Value();
        const std::string mKey = info[1].As<Napi::String>().Utf8Value();
        const std::string mId = info[2].As<Napi::String>().Utf8Value();
        client_ = std::make_shared<S63Client>(hwId, mKey, mId);
    }

    class ClientOpenWorker : public Napi::AsyncWorker {
     public:
        ClientOpenWorker(Napi::Env env, std::shared_ptr<S63Client> client, std::string path)
            : Napi::AsyncWorker(env),
              deferred_(Napi::Promise::Deferred::New(env)),
              client_(std::move(client)),
              path_(std::move(path)) {}

        Napi::Promise GetPromise() { return deferred_.Promise(); }

        void Execute() override {
            std::lock_guard<std::mutex> lock(g_s63Mutex);
            out_ = client_->open(path_);
        }

        void OnOK() override {
            deferred_.Resolve(ToBuffer(Env(), out_));
        }

        void OnError(const Napi::Error& error) override {
            deferred_.Reject(error.Value());
        }

     private:
        Napi::Promise::Deferred deferred_;
        std::shared_ptr<S63Client> client_;
        std::string path_;
        std::string out_;
    };

    class ClientDecryptAndUnzipWorker : public Napi::AsyncWorker {
     public:
        ClientDecryptAndUnzipWorker(Napi::Env env, std::shared_ptr<S63Client> client, std::string inPath,
                                    std::string outPath)
            : Napi::AsyncWorker(env),
              deferred_(Napi::Promise::Deferred::New(env)),
              client_(std::move(client)),
              inPath_(std::move(inPath)),
              outPath_(std::move(outPath)),
              usePermit_(false),
              error_(S63_ERR_DATA) {}

        ClientDecryptAndUnzipWorker(Napi::Env env, std::shared_ptr<S63Client> client, std::string inPath,
                                    std::string permit, std::string outPath)
            : Napi::AsyncWorker(env),
              deferred_(Napi::Promise::Deferred::New(env)),
              client_(std::move(client)),
              inPath_(std::move(inPath)),
              permit_(std::move(permit)),
              outPath_(std::move(outPath)),
              usePermit_(true),
              error_(S63_ERR_DATA) {}

        Napi::Promise GetPromise() { return deferred_.Promise(); }

        void Execute() override {
            std::lock_guard<std::mutex> lock(g_s63Mutex);
            if (usePermit_) {
                error_ = client_->decryptAndUnzipCell(inPath_, permit_, outPath_);
            } else {
                error_ = client_->decryptAndUnzipCell(inPath_, outPath_);
            }
        }

        void OnOK() override {
            deferred_.Resolve(Napi::Number::New(Env(), static_cast<int>(error_)));
        }

        void OnError(const Napi::Error& error) override {
            deferred_.Reject(error.Value());
        }

     private:
        Napi::Promise::Deferred deferred_;
        std::shared_ptr<S63Client> client_;
        std::string inPath_;
        std::string permit_;
        std::string outPath_;
        bool usePermit_;
        S63Error error_;
    };

    class ClientImportPermitFileWorker : public Napi::AsyncWorker {
     public:
        ClientImportPermitFileWorker(Napi::Env env, std::shared_ptr<S63Client> client, std::string path)
            : Napi::AsyncWorker(env),
              deferred_(Napi::Promise::Deferred::New(env)),
              client_(std::move(client)),
              path_(std::move(path)),
              ok_(false) {}

        Napi::Promise GetPromise() { return deferred_.Promise(); }

        void Execute() override {
            std::lock_guard<std::mutex> lock(g_s63Mutex);
            ok_ = client_->importPermitFile(path_);
        }

        void OnOK() override {
            deferred_.Resolve(Napi::Boolean::New(Env(), ok_));
        }

        void OnError(const Napi::Error& error) override {
            deferred_.Reject(error.Value());
        }

     private:
        Napi::Promise::Deferred deferred_;
        std::shared_ptr<S63Client> client_;
        std::string path_;
        bool ok_;
    };

 private:
    Napi::Value GetHWID(const Napi::CallbackInfo& info) {
        return Napi::String::New(info.Env(), client_->getHWID());
    }

    Napi::Value GetMID(const Napi::CallbackInfo& info) {
        return Napi::String::New(info.Env(), client_->getMID());
    }

    Napi::Value GetMKEY(const Napi::CallbackInfo& info) {
        return Napi::String::New(info.Env(), client_->getMKEY());
    }

    Napi::Value SetHWID(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        if (info.Length() != 1 || !info[0].IsString()) {
            return ThrowTypeError(env, "setHWID(hwId) expects 1 string argument");
        }

        client_->setHWID(info[0].As<Napi::String>().Utf8Value());
        return env.Undefined();
    }

    Napi::Value SetMID(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        if (info.Length() != 1 || !info[0].IsString()) {
            return ThrowTypeError(env, "setMID(mId) expects 1 string argument");
        }

        client_->setMID(info[0].As<Napi::String>().Utf8Value());
        return env.Undefined();
    }

    Napi::Value SetMKEY(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        if (info.Length() != 1 || !info[0].IsString()) {
            return ThrowTypeError(env, "setMKEY(mKey) expects 1 string argument");
        }

        client_->setMKEY(info[0].As<Napi::String>().Utf8Value());
        return env.Undefined();
    }

    Napi::Value InstallCellPermit(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        if (info.Length() != 1 || !info[0].IsString()) {
            return ThrowTypeError(env, "installCellPermit(cellPermit) expects 1 string argument");
        }

        return Napi::Boolean::New(env, client_->installCellPermit(info[0].As<Napi::String>().Utf8Value()));
    }

    Napi::Value ImportPermitFile(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        if (info.Length() != 1 || !info[0].IsString()) {
            return ThrowTypeError(env, "importPermitFile(path) expects 1 string argument");
        }

        bool ok;
        {
            std::lock_guard<std::mutex> lock(g_s63Mutex);
            ok = client_->importPermitFile(info[0].As<Napi::String>().Utf8Value());
        }
        return Napi::Boolean::New(env, ok);
    }

    Napi::Value ImportPermitFileAsync(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        if (info.Length() != 1 || !info[0].IsString()) {
            return ThrowTypeError(env, "importPermitFileAsync(path) expects 1 string argument");
        }

        auto* worker = new ClientImportPermitFileWorker(env, client_, info[0].As<Napi::String>().Utf8Value());
        Napi::Promise promise = worker->GetPromise();
        worker->Queue();
        return promise;
    }

    Napi::Value GetUserpermit(const Napi::CallbackInfo& info) {
        return Napi::String::New(info.Env(), client_->getUserpermit());
    }

    Napi::Value Open(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        if (info.Length() != 1 || !info[0].IsString()) {
            return ThrowTypeError(env, "open(path) expects 1 string argument");
        }

        std::string out;
        {
            std::lock_guard<std::mutex> lock(g_s63Mutex);
            out = client_->open(info[0].As<Napi::String>().Utf8Value());
        }
        return ToBuffer(env, out);
    }

    Napi::Value OpenAsync(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        if (info.Length() != 1 || !info[0].IsString()) {
            return ThrowTypeError(env, "openAsync(path) expects 1 string argument");
        }

        auto* worker = new ClientOpenWorker(env, client_, info[0].As<Napi::String>().Utf8Value());
        Napi::Promise promise = worker->GetPromise();
        worker->Queue();
        return promise;
    }

    Napi::Value DecryptAndUnzipCell(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        if (info.Length() != 2 || !info[0].IsString() || !info[1].IsString()) {
            return ThrowTypeError(env, "decryptAndUnzipCell(inPath, outPath) expects 2 string arguments");
        }

        S63Error error;
        {
            std::lock_guard<std::mutex> lock(g_s63Mutex);
            error = client_->decryptAndUnzipCell(
                    info[0].As<Napi::String>().Utf8Value(),
                    info[1].As<Napi::String>().Utf8Value());
        }
        return Napi::Number::New(env, static_cast<int>(error));
    }

    Napi::Value DecryptAndUnzipCellAsync(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        if (info.Length() != 2 || !info[0].IsString() || !info[1].IsString()) {
            return ThrowTypeError(env, "decryptAndUnzipCellAsync(inPath, outPath) expects 2 string arguments");
        }

        auto* worker = new ClientDecryptAndUnzipWorker(env, client_, info[0].As<Napi::String>().Utf8Value(),
                                                       info[1].As<Napi::String>().Utf8Value());
        Napi::Promise promise = worker->GetPromise();
        worker->Queue();
        return promise;
    }

    Napi::Value DecryptAndUnzipCellWithPermit(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        if (info.Length() != 3 || !info[0].IsString() || !info[1].IsString() || !info[2].IsString()) {
            return ThrowTypeError(env, "decryptAndUnzipCellWithPermit(inPath, cellPermit, outPath) expects 3 string arguments");
        }

        S63Error error;
        {
            std::lock_guard<std::mutex> lock(g_s63Mutex);
            error = client_->decryptAndUnzipCell(
                    info[0].As<Napi::String>().Utf8Value(),
                    info[1].As<Napi::String>().Utf8Value(),
                    info[2].As<Napi::String>().Utf8Value());
        }
        return Napi::Number::New(env, static_cast<int>(error));
    }

    Napi::Value DecryptAndUnzipCellWithPermitAsync(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        if (info.Length() != 3 || !info[0].IsString() || !info[1].IsString() || !info[2].IsString()) {
            return ThrowTypeError(env, "decryptAndUnzipCellWithPermitAsync(inPath, cellPermit, outPath) expects 3 string arguments");
        }

        auto* worker = new ClientDecryptAndUnzipWorker(env, client_, info[0].As<Napi::String>().Utf8Value(),
                                                       info[1].As<Napi::String>().Utf8Value(),
                                                       info[2].As<Napi::String>().Utf8Value());
        Napi::Promise promise = worker->GetPromise();
        worker->Queue();
        return promise;
    }

    std::shared_ptr<S63Client> client_;
};

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set("validateCellPermit", Napi::Function::New(env, ValidateCellPermitWrapped));
    exports.Set("createUserPermit", Napi::Function::New(env, CreateUserPermitWrapped));
    exports.Set("extractHwIdFromUserpermit", Napi::Function::New(env, ExtractHwIdFromUserPermitWrapped));
    exports.Set("createCellPermit", Napi::Function::New(env, CreateCellPermitWrapped));
    exports.Set("extractCellKeysFromCellpermit", Napi::Function::New(env, ExtractCellKeysFromCellPermitWrapped));
    exports.Set("decryptCellFromPath", Napi::Function::New(env, DecryptCellFromPathWrapped));
    exports.Set("decryptCellFromPathAsync", Napi::Function::New(env, DecryptCellFromPathAsyncWrapped));
    exports.Set("decryptCellBuffer", Napi::Function::New(env, DecryptCellBufferWrapped));
    exports.Set("decryptCellBufferAsync", Napi::Function::New(env, DecryptCellBufferAsyncWrapped));
    exports.Set("encryptCellBuffer", Napi::Function::New(env, EncryptCellBufferWrapped));
    exports.Set("decryptAndUnzipCellByKey", Napi::Function::New(env, DecryptAndUnzipCellByKeyWrapped));
    exports.Set("decryptAndUnzipCellByKeyAsync", Napi::Function::New(env, DecryptAndUnzipCellByKeyAsyncWrapped));

    Napi::Object errors = Napi::Object::New(env);
    errors.Set("S63_ERR_OK", Napi::Number::New(env, static_cast<int>(S63_ERR_OK)));
    errors.Set("S63_ERR_FILE", Napi::Number::New(env, static_cast<int>(S63_ERR_FILE)));
    errors.Set("S63_ERR_DATA", Napi::Number::New(env, static_cast<int>(S63_ERR_DATA)));
    errors.Set("S63_ERR_PERMIT", Napi::Number::New(env, static_cast<int>(S63_ERR_PERMIT)));
    errors.Set("S63_ERR_KEY", Napi::Number::New(env, static_cast<int>(S63_ERR_KEY)));
    errors.Set("S63_ERR_ZIP", Napi::Number::New(env, static_cast<int>(S63_ERR_ZIP)));
    errors.Set("S63_ERR_CRC", Napi::Number::New(env, static_cast<int>(S63_ERR_CRC)));
    exports.Set("S63Error", errors);

    Napi::Function s63ClientCtor = S63ClientWrap::Init(env);
    exports.Set("S63Client", s63ClientCtor);

    return exports;
}

}  // namespace

NODE_API_MODULE(addon, Init)