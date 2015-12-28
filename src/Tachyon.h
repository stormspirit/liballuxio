/**
 *  @author        Ryan Huang <ryanhuang@cs.ucsd.edu>
 *  @organization  University of California, San Diego
 * 
 * Tachyon C/C++ APIs
 *
 */

#ifndef __TACHYON_H_
#define __TACHYON_H_

#include<jni.h>
#include<stdint.h>

#include "JNIHelper.h"

#define BBUF_CLS                    "java/nio/ByteBuffer"

#define TURI_CLS                    "tachyon/TachyonURI"
#define TFS_CLS                     "tachyon/client/TachyonFS"
#define TFILE_CLS                   "tachyon/client/TachyonFile"
#define TBBUF_CLS                   "tachyon/client/TachyonByteBuffer"
#define TREADT_CLS                  "tachyon/client/ReadType"
#define TWRITET_CLS                 "tachyon/client/WriteType"

// InStream
#define TISTREAM_CLS                "tachyon/client/InStream"
#define TFILE_ISTREAM_CLS           "tachyon/client/FileInStream"
#define TBLOCK_ISTREAM_CLS          "tachyon/client/BlockInStream"
#define TLOCAL_BLOCK_ISTREAM_CLS    "tachyon/client/LocalBlockInStream"
#define TREMOTE_BLOCK_ISTREAM_CLS   "tachyon/client/RemoteBlockInStream"
#define TEMPTY_BLOCK_ISTREAM_CLS    "tachyon/client/EmptyBlockInStream"

// OutStream
#define TOSTREAM_CLS                "tachyon/client/OutStream"
#define TFILE_OSTREAM_CLS           "tachyon/client/FileOutStream"
#define TBLOCK_OSTREAM_CLS          "tachyon/client/BlockOutStream"

// non-standard Tachyon API
#define TKV_CLS                     "tachyon/client/TachyonKV"

#define DEFAULT_KV_BLOCK_BYTES      8 * 1024 * 1024 // 8MB

namespace tachyon {

enum ReadType {
  NO_CACHE,
  CACHE,
  CACHE_PROMOTE,
};

enum WriteType {
  ASYNC_THROUGH,
  CACHE_THROUGH,
  MUST_CACHE,
  THROUGH,
  TRY_CACHE,
};

class TachyonClient;
class TachyonFile;
class TachyonByteBuffer;
class TachyonURI;
class TachyonKV; // non-standard tachyon API

class ByteBuffer;
class InStream;
class OutStream;

typedef TachyonClient* jTachyonClient;
typedef TachyonFile* jTachyonFile;
typedef TachyonByteBuffer* jTachyonByteBuffer;
typedef TachyonURI* jTachyonURI;
typedef TachyonKV* jTachyonKV; // non-standard tachyon API

typedef ByteBuffer* jByteBuffer;
typedef InStream* jInStream;
typedef OutStream* jOutStream;

class JNIObjBase {
  public:
    JNIObjBase(jni::Env env, jobject localObj): m_env(env) {
      m_obj = env->NewGlobalRef(localObj);
      // this means after the constructor, the localObj will be destroyed
      env->DeleteLocalRef(localObj);
    }

    ~JNIObjBase() { m_env->DeleteGlobalRef(m_obj); }

    jobject getJObj() { return m_obj; }
    jni::Env& getEnv() { return m_env; }

  protected:
    jni::Env m_env;
    jobject m_obj; // the underlying jobject
};

class TachyonClient : public JNIObjBase {

  public:
    static jTachyonClient createClient(const char *masterUri);
    static jTachyonClient copyClient(jTachyonClient client);

    jTachyonFile getFile(const char *path);
    jTachyonFile getFile(int fid);
    jTachyonFile getFile(int fid, bool useCachedMetadata);

    int getFileId(const char *path);

    int createFile(const char *path);
    bool mkdir(const char *path);
    bool mkdirs(const char *path, bool recursive);

    bool deletePath(const char *path, bool recursive);
    bool deletePath(int fid, bool recursive);

  private:
    // hide constructor, must be instantiated using createClient method
    TachyonClient(jni::Env env, jobject tfs) : JNIObjBase(env, tfs) {}

};

class TachyonFile : public JNIObjBase {
  public:
    TachyonFile(jni::Env env, jobject tfile) : JNIObjBase(env, tfile){}
    
    long length();

    char * getPath();
    
    bool isFile();
    bool isInMemory(); 
    bool isComplete(); 
    bool isDirectory(); 
    bool needPin();    
    bool recache();                    
    
    int hashCode();    //TODO 
    bool promoteBlock(int blockIndex); //TODO
    bool rename(const char* path);     //TODO
    char* getLocalFilename(int blockIndex); //TODO
    int getNumberOfBlocks();          //TODO
    char* toString();                 //TODO
    long getBlockSizeByte();          //TODO
    long getCreationTimeMs();         //TODO
    int getDiskReplication();         //TODO
    
    jTachyonByteBuffer readByteBuffer(int blockIndex);
    jInStream getInStream(ReadType readType);
    jOutStream getOutStream(WriteType writeType);
};

class TachyonByteBuffer : public JNIObjBase {
  public:
    TachyonByteBuffer(jni::Env env, jobject tbbuf) : JNIObjBase(env, tbbuf) {}

    jByteBuffer getData();
    void close();

};

class ByteBuffer : public JNIObjBase {
  public:
    ByteBuffer(jni::Env env, jobject bbuf): JNIObjBase(env, bbuf){}

    static jByteBuffer allocate(int capacity);
};

class InStream : public JNIObjBase {
  public:
    InStream(jni::Env env, jobject istream): JNIObjBase(env, istream){}
  
    void close();
    int read();
    int read(void *buff, int length);
    int read(void *buff, int length, int off, int maxLen);
    void seek(long pos);
    long skip(long n);
  
};

class FileInStream : public InStream {
};

class EmptyBlockInStream : public InStream {
};

class BlockInStream : public InStream {
};

class LocalBlockInStream : public BlockInStream {
};

class RemoteBlockInStream : public BlockInStream {
};

class OutStream : public JNIObjBase {
  public:
    OutStream(jni::Env env, jobject ostream): JNIObjBase(env, ostream){}

    void cancel();
    void close();
    void flush();
    void write(int byte);
    void write(const void *buff, int length);
    void write(const void *buff, int length, int off, int maxLen);

};

class FileOutStream : public OutStream {
};

class BlockOutStream : public OutStream { 
    bool canWrite(); //TODO
    long getBlockId(); //TODO
    long getBlockOffset(); //TODO
    long getRemainingSpaceByte(); //TODO
};

class TachyonURI : public JNIObjBase {
  public:
    static jTachyonURI newURI(const char *pathStr);
    static jTachyonURI newURI(const char *scheme, const char *authority, const char *path);
    static jTachyonURI newURI(jTachyonURI parent, jTachyonURI child);

    TachyonURI(jni::Env env, jobject uri): JNIObjBase(env, uri){}
};

// non-standard tachyon API
class TachyonKV : public JNIObjBase {
  public:
    ~TachyonKV() {
      if (m_client != NULL) {
        delete m_client;
      }
    }

    static jTachyonKV createKV(jTachyonClient client, ReadType readType, 
        WriteType writeType, long blockSizeByte = DEFAULT_KV_BLOCK_BYTES, 
        const char *kvStore = NULL);

  
    bool init(); // test if kv store can be initialized before doing r/w
    int get(const char *key, uint32_t keylen, char *buff, uint32_t valuelen);
    void set(const char *key, uint32_t keylen, const char *buff, uint32_t valuelen);
    jTachyonClient getClient() { return m_client; }

  private:
    TachyonKV(jTachyonClient client, jobject tkv) : JNIObjBase(client->getEnv(), tkv) {
      m_client = TachyonClient::copyClient(client);
    } 

    jTachyonClient m_client;
};



} // namespace tachyon

#ifdef __cplusplus
extern "C" {
#endif

jobject enumObjReadType(tachyon::jni::Env& env, tachyon::ReadType readType);
jobject enumObjWriteType(tachyon::jni::Env& env, tachyon::WriteType writeType);

char* fullTachyonPath(const char *masterUri, const char *filePath);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __TACHYON_H_ */

/* vim: set ts=4 sw=4 : */
