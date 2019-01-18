#ifndef __HIREDIS_CLIENT_H__
#define __HIREDIS_CLIENT_H__

#include <hiredis/hiredis.h>
//#include <hiredis/async.h>
//#include <hiredis/adapters/libevent.h>
#include <string>
#include <vector>
#include <map>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/lexical_cast.hpp>


namespace demo {
namespace hiredis {

template <class T>
static std::string type2str(T v) {
    std::stringstream s;
    s << v;

    return s.str();
}

enum ReplyType
{
    STRING    = 1,
    ARRAY     = 2,
    INTEGER   = 3,
    NIL       = 4,
    STATUS    = 5,
    ERROR     = 6
};

//unlikely or like only supported by c11
//if use symbols in boost, modify followed macro define

#define DY_UNLIKELY(expr)  (expr)
#define DY_LIKELY(expr)  (expr)
//#define DY_UNLIKELY(expr)  unlikely(expr)
//#define DY_LIKELY(expr)    likely(expr)

class Reply
{
  public:
    ReplyType Type() const {return type_;}
    long long Integer() const {return integer_;}
    const std::string& Str() const {return str_;}
    const std::vector<Reply>& Elements() const {return elements_;}

    Reply(redisReply *reply = NULL):type_(ERROR), integer_(0)
    {
        if (reply == NULL)
            return;

        type_ = static_cast<ReplyType>(reply->type);
        switch(type_) {
            case ERROR:
            case STRING:
            case STATUS:
                str_ = std::string(reply->str, reply->len);
                break;
            case INTEGER:
                integer_ = reply->integer;
                break;
            case ARRAY:
                for (size_t i = 0; i < reply->elements; ++i){
                    elements_.push_back(Reply(reply->element[i]));
                }
                break;
            default:
                break;
        }
    }

    ~Reply(){}

    void Print() const {
        if (Type() == NIL) {
            printf("NIL.\n");
        }
        if (Type() == STRING) {
            printf("STRING:%s\n", Str().c_str());
        }
        if (Type() == ERROR) {
            printf("ERROR:%s\n", Str().c_str());
        }
        if (Type() == STATUS) {
            printf("STATUS:%s\n", Str().c_str());
        }
        if (Type() == INTEGER) {
            printf("INTEGER:%lld\n", Integer());
        }
        if (Type() == ARRAY) {
            const std::vector<Reply>& elements = Elements();

            for (size_t j = 0; j != elements.size(); j++) {
                printf("%lu) ", j);
                elements[j].Print();
            }
        }
    }

    // 用于调试，勿频繁调用
    std::string to_string() const
	{
        std::stringstream os;
		switch (Type())
		{
			case demo::hiredis::NIL:
				os << "NIL" << std::endl;
				break;
			case demo::hiredis::STRING:
				os << "STRING:" << Str() << std::endl;
				break;
			case demo::hiredis::ERROR:
				os << "ERROR:" << Str() << std::endl;
				break;
			case demo::hiredis::STATUS:
				os << "STATUS:" << Str() << std::endl;
				break;
			case demo::hiredis::INTEGER:
				os << "INTEGER:" << Integer() << std::endl;
				break;
			case demo::hiredis::ARRAY:
				const std::vector<demo::hiredis::Reply>& elements = Elements();
				for (size_t i = 0; i != elements.size(); i++)
				{
					os << i << ")";
					os << elements[i].to_string();
				}
        } 
        return os.str();
    }

  private:
    ReplyType           type_;
    std::string         str_;
    long long           integer_;
    std::vector<Reply>  elements_;
};

class Client: boost::noncopyable
{
public:
    Client()
        :host_("localhost"),port_(6379),pwd_(""),db_(0)
        ,connect_timeout_(3000),snd_rcv_timeout_(10000),srv_timeout_(3),use_time_(0),rc_(NULL)
    {
        // Init("localhost", 6379, "", 0);
    }
    Client(const std::string& host, int32_t port,
           const std::string& pwd,  int32_t db)
        :host_(host),port_(port),pwd_(pwd),db_(db)
        ,connect_timeout_(3000),snd_rcv_timeout_(10000),srv_timeout_(3),use_time_(0),rc_(NULL)
    {
        // Init(host, port, pwd, db);
    }
    int32_t Init(const std::string& host, int32_t port,
                 const std::string& pwd,  int32_t db)
    {
        host_   = host;
        port_   = port;
        pwd_    = pwd;
        db_     = db;

        return Connect();
    }

    /*
    int32_t Init(const MylibIni& ini, const std::string& section = "redis") 
    {
        host_   = ini.get(section, "host", "127.0.0.1");
        port_   = ini.get(section, "port", 6379);
        pwd_    = ini.get(section, "pwd", "");
        db_     = ini.get(section, "db",   0);

        return Connect();
    }
    */
    ~Client()
    {
        Close();
    }

    bool CheckConnect()
    {        
        if(NULL == rc_){
            if(0 != Connect())
                return false;
        }
        else{
            time_t now = time(NULL);
            if(now - use_time_ >= srv_timeout_ && 0 != Ping() && 0 != Connect())
                return false;
        }

        return true;
    }

    int32_t Command(const std::vector<std::string> &send, Reply &reply)
    {
        //if(NULL == rc_)
        //{
        //     errmsg_="redis client is not connected";
        //    return REDIS_ERR;
        //}
        // 发起redis调用

        if(!CheckConnect()) //定时检查连接
        {
            return REDIS_ERR;
        }
            

        int32_t     ret         = REDIS_OK;
        redisReply  *c_reply    = NULL;

        std::vector<const char*> argv;
        std::vector<size_t> argvlen;
        argv.reserve(send.size());
        argvlen.reserve(send.size());

        for(std::vector<std::string>::const_iterator it = send.begin();
            it != send.end();
            ++it) {
            argv.push_back(it->c_str());
            argvlen.push_back(it->size());
        }

        c_reply = (redisReply *)redisCommandArgv(rc_, static_cast<int>(send.size()), argv.data(), argvlen.data());
        if (c_reply == NULL) {
            //失败时立即重连一次
            if(0 != Connect())
            {
                return REDIS_ERR;
            }            

            c_reply = (redisReply *)redisCommandArgv(rc_, static_cast<int>(send.size()), argv.data(), argvlen.data());
        }

        if(c_reply == NULL){
            ret = rc_->err;
            errmsg_.clear();
            errmsg_.append("Command:");
            errmsg_.append("[").append(ErrorToStr(ret)).append("]");
            errmsg_.append(std::string(rc_->errstr));

            return ret;
        }
        
        reply = Reply(c_reply);
        freeReplyObject(c_reply);
        use_time_ = time(NULL);
        
        return ret;
    }

    const std::string& ErrMsg()
    {
        return errmsg_;
    }

    void SetHost(const std::string& host)
    {
        host_ = host;
        return;
    }

    void SetPort(int32_t port)
    {
        port_ = port;
        return;
    }

    void SetPass(const std::string& pass)
    {
        pwd_ = pass;
        return;
    }

    void SetDB(int32_t db)
    {
        db_ = db;
        return;
    }

    int32_t GetDB()
    {
        return db_;
    }

    int32_t SetTimeout(int64_t connect_timeout, int64_t snd_rcv_timeout)
    {
        connect_timeout_ = connect_timeout;
        snd_rcv_timeout_ = snd_rcv_timeout;

        return SetRedisContextTimeout();
    }
    void GetTimeout(int64_t& connect_timeout, int64_t& snd_rcv_timeout)
    {
        connect_timeout = connect_timeout_;
        snd_rcv_timeout = snd_rcv_timeout_;
    }

private:
    static const std::string& ErrorToStr(int32_t errorno)
    {
        static std::string redis_err_io         = "REDIS_ERR_IO";
        static std::string redis_err_eof        = "REDIS_ERR_EOF";
        static std::string redis_err_protocol   = "REDIS_ERR_PROTOCOL";
        static std::string redis_err_oom        = "REDIS_ERR_OOM";
        static std::string redis_err_other      = "REDIS_ERR_OTHER";

        if (errorno == REDIS_ERR_IO) {
            return redis_err_io;
        }
        else if (errorno == REDIS_ERR_EOF) {
            return redis_err_eof;
        }
        else if (errorno == REDIS_ERR_PROTOCOL) {
            return redis_err_protocol;
        }
        else if (errorno == REDIS_ERR_OOM) {
            return redis_err_oom;
        }
        else {
            return redis_err_other;
        }
    }

private:
    class ReplyHandle {//could also use shared_ptr instead
    public:
        ReplyHandle(redisReply *raw_reply)
            :raw_reply_(raw_reply) 
        {

        }
        ~ReplyHandle()
        {
            if (DY_LIKELY(NULL != raw_reply_)) {
                freeReplyObject(raw_reply_);
                raw_reply_ = NULL;
            }
        }
        redisReply* Reply() {
            return raw_reply_;
        }
    private:
        redisReply *raw_reply_;
    };

private:
    int32_t Ping()
    {
        if (DY_UNLIKELY(NULL == rc_))
            return -1;

        ReplyHandle reply_handle((redisReply*)redisCommand(rc_, "ping"));
        if (DY_UNLIKELY(NULL == reply_handle.Reply())) {
            return -1;
        }
        if (DY_UNLIKELY(ERROR == reply_handle.Reply()->type)) {
            errmsg_ = rc_->errstr;
            return -2;
        }

        // avoid coredump add by zhengwei date
		int reply_type = reply_handle.Reply()->type;
		if (reply_type != REDIS_REPLY_ERROR 
                && reply_type != REDIS_REPLY_STRING
                && reply_type != REDIS_REPLY_STATUS) {
			return -3;
		}

        if (NULL == reply_handle.Reply()->str) {
            return -4;
        }

        if (DY_UNLIKELY(std::string("PONG") != std::string(reply_handle.Reply()->str,reply_handle.Reply()->len) &&
                     std::string("pong") != std::string(reply_handle.Reply()->str,reply_handle.Reply()->len))) {
            errmsg_ = rc_->errstr;
            return -5;
        }

        return 0;
    }

    int32_t Auth()
    {
        if (DY_UNLIKELY(NULL == rc_))
            return -1;
        if (DY_UNLIKELY(pwd_.empty()))
            return 0;

        ReplyHandle reply_handle(((redisReply*)redisCommand(rc_, "auth %s", pwd_.c_str())));
        if (DY_UNLIKELY(NULL == reply_handle.Reply())) {
            return -1;
        }
        if (DY_UNLIKELY(ERROR == reply_handle.Reply()->type)) {
            errmsg_ = rc_->errstr;
            return -2;
        }
        if (DY_UNLIKELY(std::string("OK") != std::string(reply_handle.Reply()->str,reply_handle.Reply()->len) &&
                     std::string("ok") != std::string(reply_handle.Reply()->str,reply_handle.Reply()->len))) {
            errmsg_ = rc_->errstr;
            return -3;
        }

        return 0;
    }

    int32_t Select()
    {
        if (DY_UNLIKELY(NULL == rc_))
            return -1;
        ReplyHandle reply_handle((redisReply*)redisCommand(rc_, "select %d", db_));
        if (DY_UNLIKELY(NULL == reply_handle.Reply())) {
            return -1;
        }
        if (DY_UNLIKELY(ERROR == reply_handle.Reply()->type)) {
            errmsg_ = rc_->errstr;
            return -2;
        }
        if (DY_UNLIKELY(std::string("OK") != std::string(reply_handle.Reply()->str,reply_handle.Reply()->len) &&
                     std::string("ok") != std::string(reply_handle.Reply()->str,reply_handle.Reply()->len))) {
            errmsg_ = rc_->errstr;
            return -3;
        }

        return 0;
    }

    int32_t ConnectImpl()
    {
        struct timeval tv_conn;
        tv_conn.tv_sec  = connect_timeout_/1000;        //ms convert to second
        tv_conn.tv_usec = 1000*(connect_timeout_%1000); //ms convert to microsecond

        rc_ = redisConnectWithTimeout(host_.c_str(), port_, tv_conn);
        if (DY_UNLIKELY(rc_ == NULL || rc_->err != REDIS_OK)) {
            if(rc_) {
                errmsg_ = rc_->errstr;
                Close(); 
            }               
            else
                errmsg_ = "redisContext is null";

            return -1;
        }

        redisEnableKeepAlive(rc_);//ignore fail

        return 0;
    }

    int32_t SetRedisContextTimeout()
    {
        if (DY_UNLIKELY(NULL == rc_))
            return -1;
        struct timeval tv_rw;
        tv_rw.tv_sec  = snd_rcv_timeout_/1000;          //ms convert to second
        tv_rw.tv_usec = 1000*(snd_rcv_timeout_%1000);   //ms convert to microsecond
        if (DY_UNLIKELY(0 != redisSetTimeout(rc_, tv_rw))) {
            errmsg_ = rc_->errstr;
            return -1;
        }

        return 0;
    }

    int32_t Connect()
    {
        Close();

        if (DY_UNLIKELY(0 != ConnectImpl()))
            return -1;

        if (DY_UNLIKELY(0 != SetRedisContextTimeout() ||
                     0 != Auth() ||
                     0 != Select())) {
            Close();
            return -2;
        }

        use_time_ = time(NULL);
        return 0;
    }

    void Close()
    {
        if (rc_ != NULL) {
            redisFree(rc_);
            rc_ = NULL;
        }
    }

private:
    std::string     host_;
    int32_t         port_;
    std::string     pwd_;
    int32_t         db_;

    int64_t         connect_timeout_;//ms
    int64_t         snd_rcv_timeout_;//ms

    int32_t         srv_timeout_; //s
    int32_t         use_time_;

    std::string     errmsg_;
    redisContext    *rc_;
};

class ServiceBase
{
public:
    ServiceBase(Client& client) : client_(client), old_db_(-1){}
    ~ServiceBase(){}

    const std::string& ErrMsg()
    {
        return errmsg_;
    }

    void RecordErrmsg(const std::string& errmsg)
    {
        errmsg_ = errmsg;
    }

    bool SetDB(int32_t db) {
        old_db_ = client_.GetDB();
        client_.SetDB(db);
        return SelectDB(db);
    }

    bool ResetDB() {
        if (old_db_ != -1) {
            client_.SetDB(old_db_);
            return SelectDB(old_db_);
        }

        return true;
    }

protected:
    bool SelectDB(int32_t db)
    {
        std::vector<std::string> send;
        send.push_back("select");
        send.push_back(type2str(db));

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        return true;
    }

protected:
    std::string errmsg_;
    Client& client_;
    int32_t old_db_;
};

// class SystemService : public ServiceBase
// {
// public:
//     SystemService(Client& client) : ServiceBase(client){}
//     ~SystemService(){}

//     //from 0 to ...
//     bool SelectDB(const std::string& index)
//     {
//         std::vector<std::string> send;
//         send.push_back("select");
//         send.push_back(index);

//         Reply reply;
//         int32_t ret = client_.Command(send, reply);
//         if (ret != REDIS_OK) {
//             RecordErrmsg(client_.ErrMsg());
//             return false;
//         }

//         if (reply.Type() == ERROR) {
//             RecordErrmsg(reply.Str());
//             return false;
//         }

//         return true;
//     }
// };

class RedisHash : public ServiceBase
{
public:
    RedisHash(Client& client) : ServiceBase(client){}
    ~RedisHash(){}

    bool hGet(const std::string& hash, const std::string& field, std::string& value)
    {
        std::vector<std::string> send;
        send.push_back("hget");
        send.push_back(hash);
        send.push_back(field);

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        if (reply.Type() == ERROR) {
            RecordErrmsg(reply.Str());
            return false;
        }

        value = reply.Str();
        return true;
    }

    bool hSet(const std::string& hash, const std::string& field, const std::string& value)
    {
        std::vector<std::string> send;
        send.push_back("hset");
        send.push_back(hash);
        send.push_back(field);
        send.push_back(value);

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        if (reply.Type() == ERROR) {
            RecordErrmsg(reply.Str());
            return false;
        }

        return true;
    }

    bool hDel(const std::string& hash, const std::string& field)
    {
        std::vector<std::string> send;
        send.push_back("hdel");
        send.push_back(hash);
        send.push_back(field);

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        if (reply.Type() == ERROR) {
            RecordErrmsg(reply.Str());
            return false;
        }

        if ( reply.Integer() == 1 ) {
            return true;
        }

        RecordErrmsg("member not exist");
        return false;
    }

    bool hKeys(const std::string& hash, std::vector<std::string>& item_vec)
    {
        std::vector<std::string> send;
        send.push_back("hkeys");
        send.push_back(hash);

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }
        if (reply.Type() != ARRAY) {
            RecordErrmsg("array result expected");
            return false;
        }

        item_vec.clear();
        const std::vector<Reply>& element_vec = reply.Elements();
        for (size_t i = 0; i != element_vec.size(); ++i) {
            item_vec.push_back(element_vec[i].Str());
        }
        return true;
    }

    bool hScan(const std::string& hash,
            const std::string& match,
            const std::string& count,
            std::string& cursor,
            std::vector<std::pair<std::string,std::string> >& item_vec)
    {
        std::vector<std::string> send;
        send.push_back("hscan");
        send.push_back(hash);
        if (cursor.size() <= 0)
        {
            RecordErrmsg("invalid hscan cursor");
            return false;
        }

        send.push_back(cursor);

        if (match.size() > 0)
        {
            send.push_back("match");
            send.push_back(match);
        }
        if (count.size() > 0)
        {
            send.push_back("count");
            send.push_back(count);
        }

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK)
        {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        if (reply.Type() != ARRAY)
        {
            RecordErrmsg("array result expected");
            return false;
        }

        const std::vector<Reply>& element_vec = reply.Elements();
        if (element_vec.size() != 2
                || element_vec[1].Type() != ARRAY)
        {
            RecordErrmsg("element[1] array result expected");
            return false;
        }

        cursor = element_vec[0].Str();
        const std::vector<Reply>& result_vec = element_vec[1].Elements();
        for (size_t i = 0; i < result_vec.size(); i += 2)
        {
            item_vec.push_back(make_pair(result_vec[i].Str(), result_vec[i + 1].Str()));
        }

        return true;
    }

    //bool hGetAll(const std::string& hash, std::vector<std::string>& item_vec)
    bool hGetAll(const std::string& hash, std::vector<std::pair<std::string,std::string> >& item_vec)
    {
        std::vector<std::string> send;
        send.push_back("hgetall");
        send.push_back(hash);

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        if (reply.Type() != ARRAY) {
            RecordErrmsg("array result expected");
            return false;
        }

        item_vec.clear();
        const std::vector<Reply>& element_vec = reply.Elements();
        for (size_t i = 0; i < element_vec.size(); i+=2) {
            //item_vec.push_back(make_pair(s[i], s[i + 1]));
            item_vec.push_back(make_pair(element_vec[i].Str(),element_vec[i+1].Str()));
        }

        return true;
    }

    bool hMGet(const std::string& hash, std::vector<std::string>& vec_keys, std::map<std::string, std::string>& map_values)
    {
        std::vector<std::string> send;
        send.push_back("hmget");
        send.push_back(hash);
        std::vector<std::string>::iterator iter;

        for (iter = vec_keys.begin(); iter != vec_keys.end(); ++iter)
        {
            send.push_back(*iter);
        }

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        if (reply.Type() != ARRAY) {
            RecordErrmsg("array result expected");
            return false;
        }

        const std::vector<Reply>& element_vec = reply.Elements();
        if (element_vec.size() != vec_keys.size())
        {
            RecordErrmsg("result size is not equal keys size!");
            return false;
        }
        for (size_t i = 0, len = element_vec.size(); i < len; ++i) {
            map_values.insert(make_pair(vec_keys[i], element_vec[i].Str()));
        }

        return true;
    }

    bool hMSet(const std::string& hash, const std::map<std::string, std::string>& map_values)
    {
        std::vector<std::string> send;
        send.push_back("hmset");
        send.push_back(hash);
        std::map<std::string, std::string>::const_iterator iter;

        for (iter = map_values.begin(); iter != map_values.end(); ++iter)
        {
            send.push_back(iter->first);
            send.push_back(iter->second);
        }

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        if (reply.Type() == ERROR) {
            RecordErrmsg(reply.Str());
            return false;
        }

        return true;
    }
    
    bool hIncrby(const std::string& hash, const std::string& field, int64_t incr, int64_t& result)
    {
        std::vector<std::string> send;
        send.push_back("HINCRBY");
        send.push_back(hash);
        send.push_back(field);
        send.push_back(type2str(incr));

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        if (reply.Type() == ERROR) {
            RecordErrmsg(reply.Str());
            return false;
        }

        result = reply.Integer();
        return true;
    }

    bool hExists(const std::string& hash, const std::string& field, int32_t & exist)
    {
        std::vector<std::string> send;
        send.push_back("HEXISTS");
        send.push_back(hash);
        send.push_back(field);

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        if (reply.Type() == ERROR) {
            RecordErrmsg(reply.Str());
            return false;
        }

        exist = reply.Integer();
        return true;
    }
};

class RedisList : public ServiceBase
{
public:
    RedisList(Client& client) : ServiceBase(client){}
    ~RedisList(){}

    bool lPop(const std::string& list, std::string& value)
    {
        return Pop(list, "lpop", value);
    }

    bool rPop(const std::string& list, std::string& value)
    {
        return Pop(list, "rpop", value);
    }

    bool lPush(const std::string& list, const std::string& value)
    {
        return PushImpl(list, "lpush", value);
    }

    bool RPopLPush(const std::string& list_src, const std::string& list_dst, std::string& value)
    {
        std::vector<std::string> send;
        send.push_back("RPopLPush");
        send.push_back(list_src);
        send.push_back(list_dst);

        std::string key;
        return PopImpl(send, key, value);
    }
    bool LRange(const std::string& key, int32_t start, int32_t stop, std::vector<std::string>& item_vec)
    {
        std::vector<std::string> send;
        send.push_back("LRange");
        send.push_back(key);
        send.push_back(type2str(start));
        send.push_back(type2str(stop));

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }
        if (reply.Type() != ARRAY) {
            RecordErrmsg("array result expected");
            return false;
        }

        item_vec.clear();
        const std::vector<Reply>& element_vec = reply.Elements();
        for (size_t i = 0; i != element_vec.size(); ++i) {
            item_vec.push_back(element_vec[i].Str());
        }
        return true;
    }
    bool LTrim(const std::string& key, int32_t start, int32_t stop)
    {
        std::vector<std::string> send;
        send.push_back("LTrim");
        send.push_back(key);
        send.push_back(type2str(start));
        send.push_back(type2str(stop));

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }
        if (reply.Type() != STATUS || reply.Str() != "OK") {
            RecordErrmsg("STATUS [OK] result expected");
            return false;
        }
        return true;
    }

    bool rPush(const std::string& list, const std::string& value)
    {
        return PushImpl(list, "rpush", value);
    }

    bool lLen(const std::string& list, int64_t& len)
    {
        std::vector<std::string> send;
        send.push_back("llen");
        send.push_back(list);

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        if (reply.Type() != INTEGER) {
            RecordErrmsg(reply.Str());
            return false;
        }

        len = (int64_t)reply.Integer();
        return true;
    }

    bool Remove(const std::string& list, const std::string& item)
    {
        std::vector<std::string> send;
        send.push_back("LREM");
        send.push_back(list);
        send.push_back("-1");    //remove 1 from tail to head
        send.push_back(item);

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        if (reply.Type() != INTEGER) {
            RecordErrmsg(reply.Str());
            return false;
        }

        if (0 == (int64_t)reply.Integer()) {//nothing removed
            RecordErrmsg("item not found");
        }
        return true;
    }

    bool BRPop(const std::string& list, std::string& value, int64_t second_wait = 0)//0 for block indefinitely
    {
        return BPop(list, "BRPOP", value, second_wait);
    }

    bool BLPop(const std::string& list, std::string& value, int64_t second_wait = 0)//0 for block indefinitely
    {
        return BPop(list, "BLPOP", value, second_wait);
    }

    bool BRPopLPush(const std::string& list_src, const std::string& list_dst, std::string& value, int64_t second_wait = 0)//0 for block indefinitely
    {
        std::vector<std::string> send;
        send.push_back("BRPopLPush");
        send.push_back(list_src);
        send.push_back(list_dst);
        send.push_back(type2str(second_wait));

        std::string key;
        return PopImpl(send, key, value);
    }

protected:
    bool BPop(const std::string& list, const std::string& cmd, std::string& value, int64_t second_wait)
    {
        std::vector<std::string> send;
        send.push_back(cmd);
        send.push_back(list);
        send.push_back(type2str(second_wait));

        std::string key;//ignored for single list pop
        return PopImpl(send, key, value);
    }

    bool Pop(const std::string& list, const std::string& cmd, std::string& value)
    {
        std::vector<std::string> send;
        send.push_back(cmd);
        send.push_back(list);

        std::string key;//ignored for single list pop
        return PopImpl(send, key, value);
    }
private:
    bool PopImpl(const std::vector<std::string>& send, std::string& key, std::string& value)
    {
        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        if (reply.Type() == ERROR) {
            RecordErrmsg(reply.Str());
            return false;
        }

        if (reply.Type() == STRING) {
            value = reply.Str();
            return true;
        }

        if (reply.Type() == NIL) {//list is empty or timeout is reached
            key = value = "";
            return true;
        }

        if (reply.Type() == ARRAY) {
            const std::vector<Reply>& reply_vec = reply.Elements();
            if (reply_vec.size() != 2) {
                RecordErrmsg("reply_vec_size is less than 2");
                return false;
            }
            // if (reply_vec[0].Type() != STRING || reply_vec[1].Type() != STRING) {
            //     RecordErrmsg("reply_vec[] Type are not STRING");
            //     return false;
            // }
            key     = reply_vec[0].Str();
            value   = reply_vec[1].Str();
            return true;
        }

        RecordErrmsg("wrong reply type");
        return false;
    }

    bool PushImpl(const std::string& list, const std::string& cmd, const std::string& value)
    {
        std::vector<std::string> send;
        send.push_back(cmd);
        send.push_back(list);
        send.push_back(value);

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        if (reply.Type() == ERROR) {
            RecordErrmsg(reply.Str());
            return false;
        }

        return true;
    }
};

class RedisSets : public ServiceBase
{
public:
    RedisSets(Client& client) : ServiceBase(client){}
    ~RedisSets(){}

    bool Sadd(const std::string& sets, const std::string& value)
    {
        std::vector<std::string> send;
        send.push_back("sadd");
        send.push_back(sets);
        send.push_back(value);

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        return true;
    }

    bool Spop(const std::string& sets, std::string& value)
    {
        std::vector<std::string> send;
        send.push_back("spop");
        send.push_back(sets);

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        if (reply.Type() == ERROR) {
            RecordErrmsg(reply.Str());
            return false;
        }

        value = reply.Str();

        return true;
    }

    bool Sismember(const std::string& sets, std::string& member, bool & exist)
    {
        std::vector<std::string> send;
        send.push_back("SISMEMBER");
        send.push_back(sets);
        send.push_back(member);

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        exist = (reply.Integer() == 1);

        return true;
    }

    bool Smembers(const std::string& sets, std::vector<std::string >& members)
    {
        std::vector<std::string> send;
        send.push_back("SMEMBERS");
        send.push_back(sets);

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        if (reply.Type() != ARRAY) {
            RecordErrmsg("array result expected");
            return false;
        }

        members.clear();
        const std::vector<Reply>& element_vec = reply.Elements();
        for (size_t i = 0; i < element_vec.size(); i++) {
            members.push_back(element_vec[i].Str());
        }

        return true;
    }

    bool Scard(const std::string& sets, int64_t & size)
    {
        std::vector<std::string> send;
        send.push_back("SCARD");
        send.push_back(sets);

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        if (reply.Type() == ERROR) {
            RecordErrmsg(reply.Str());
            return false;
        }

        size = reply.Integer();
        return true;
    }
};

class RedisString : public ServiceBase
{
public:
    explicit RedisString(Client& client) : ServiceBase(client) {}
    RedisString(const std::string& key_prefix, Client& client) : ServiceBase(client),namespace_(key_prefix) {}
    ~RedisString(){}

    void SetNamespace(const std::string& key_prefix) { namespace_ = key_prefix; }
    std::string GetNamespace() const { return namespace_; }

    bool Del(const std::string& key)
    {
        std::vector<std::string> send;
        send.push_back("del");
        send.push_back(GetRealKey(key));

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        if (reply.Type() == ERROR) {
            RecordErrmsg(reply.Str());
            return false;
        }

        return true;
    }

    std::string Get(const std::string& key)
    {
        std::string result;
        Get(key, result);
        return result;
    }

    bool Get(const std::string& key, std::string& value)
    {
        std::vector<std::string> send;
        send.push_back("get");
        send.push_back(GetRealKey(key));

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        if (reply.Type() == ERROR) {
            RecordErrmsg(reply.Str());
            return false;
        }

        value = reply.Str();
        return true;
    }
    
    bool MGet(const std::vector<std::string>& keys, std::map<std::string, std::string> & values)
    {
        std::vector<std::string> send;
        send.push_back("mget");
        //send.push_back(GetRealKey(key));

        std::vector<std::string>::const_iterator iter = keys.begin();

        for (; iter != keys.end(); ++iter)
        {
            send.push_back(GetRealKey(*iter));
        }

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        if (reply.Type() != ARRAY) {
            RecordErrmsg("array result expected");
            return false;
        }

        const std::vector<Reply>& element_vec = reply.Elements();
        if (element_vec.size() != keys.size())
        {
            RecordErrmsg("result size is not equal keys size!");
            return false;
        }

        for (size_t i = 0, len = element_vec.size(); i < len; ++i) 
        {
            values.insert(make_pair(keys[i], element_vec[i].Str()));
        }

        return true;
    }

    bool TimeToLive(const std::string& key, int64_t& val)
    {
        std::vector<std::string> send;
        send.push_back("TTL");
        send.push_back(GetRealKey(key));

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        if (reply.Type() != INTEGER) {
            RecordErrmsg(reply.Str());
            return false;
        }

        val = reply.Integer();
        return true;
    }

    bool Set(const std::string& key, const std::string& value)
    {
        std::vector<std::string> send;
        send.push_back("set");
        send.push_back(GetRealKey(key));
        send.push_back(value);

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        if (reply.Type() == ERROR) {
            RecordErrmsg(reply.Str());
            return false;
        }

        return true;
    }

    bool Set(const std::string& key, const std::string& value, int64_t second)
    {
        std::vector<std::string> send;
        send.push_back("set");
        send.push_back(GetRealKey(key));
        send.push_back(value);
        send.push_back("ex");
        send.push_back(type2str(second));

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        if (reply.Type() == ERROR) {
            RecordErrmsg(reply.Str());
            return false;
        }

        return true;
    }

    int32_t Setnx(const std::string& key, const std::string& value, int64_t second)
    {
        std::vector<std::string> send;
        send.push_back("set");
        send.push_back(GetRealKey(key));
        send.push_back(value);
        send.push_back("nx");
        send.push_back("ex");
        send.push_back(type2str(second));

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return -1;
        }

        if (reply.Type() == ERROR) {
            RecordErrmsg(reply.Str());
            return -1;
        }

        if (NIL == reply.Type())
        {
            return 1;
        }

        return 0;
    }

    bool Setex(const std::string& key, const std::string& value, const std::string& second)
    {
        std::vector<std::string> send;
        send.push_back("setex");
        send.push_back(GetRealKey(key));
        send.push_back(second);
        send.push_back(value);

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        if (reply.Type() == ERROR) {
            RecordErrmsg(reply.Str());
            return false;
        }

        return true;
    }

    bool Expire(const std::string& key, const std::string& second)
    {
        std::vector<std::string> send;
        send.push_back("expire");
        send.push_back(GetRealKey(key));
        send.push_back(second);

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        if (reply.Type() == ERROR) {
            RecordErrmsg(reply.Str());
            return false;
        }

        int64_t val = reply.Integer();
        if (val == 0) {
            RecordErrmsg("expire reply return 0");
            return false;
        }

        return true;
    }

    bool Setnx(const std::string& key, const std::string& val)
    {
        std::vector<std::string> send;
        send.push_back("setnx");
        send.push_back(GetRealKey(key));
        send.push_back(val);

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        if (reply.Type() == ERROR) {
            RecordErrmsg(reply.Str());
            return false;
        }

        int64_t result = reply.Integer();
        if (result != 1) {
            RecordErrmsg("setnx return 0");
            return false;
        }

        return true;
    }

    bool Incrby(const std::string& key, const std::string& val, int64_t& result)
    {
        std::vector<std::string> send;
        send.push_back("incrby");
        send.push_back(GetRealKey(key));
        send.push_back(val);
        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }
        if (reply.Type() != INTEGER) {
            RecordErrmsg(reply.Str());
            return false;
        }
        result = reply.Integer();
        return true;
    }

    bool Expireat(const std::string& key, const std::string time_stamp)
    {
        std::vector<std::string> send;
        send.push_back("expireat");
        send.push_back(GetRealKey(key));
        send.push_back(time_stamp);
        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }
        if (reply.Type() != INTEGER) {
            RecordErrmsg(reply.Str());
            return false;
        }
        if (reply.Integer() == 0) {
            RecordErrmsg("expireat return 0, key does not exist or the timeout could not be set");
            return false;
        }
        return true;
    }

    bool Incr(const std::string& key, int64_t& val)
    {
        return IncrBy(key, 1, val);
    }
    bool IncrBy(const std::string& key, int64_t delta, int64_t& val)
    {
        std::vector<std::string> send;
        send.push_back("INCRBY");
        send.push_back(GetRealKey(key));
        send.push_back(type2str(delta));

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        if (reply.Type() != INTEGER) {
            RecordErrmsg(reply.Str());
            return false;
        }

        val = reply.Integer();
        return true;
    }
    bool Decr(const std::string& key, int64_t& val)
    {
        return DecrBy(key, 1, val);
    }
    bool DecrBy(const std::string& key, int64_t delta, int64_t& val)
    {
        std::vector<std::string> send;
        send.push_back("DECRBY");
        send.push_back(GetRealKey(key));
        send.push_back(type2str(delta));

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        if (reply.Type() == INTEGER) {
            val = reply.Integer();
            return true;
        }

        if (reply.Type() == STATUS && "QUEUED" == reply.Str()) {//transaction support
            return true;
        }

        RecordErrmsg(reply.Str());
        return false;
    }
  protected:
    std::string GetRealKey(const std::string& key) {
        if (namespace_.empty())
            return key;

        return namespace_ + key;
    }
  private:
    std::string     namespace_;
};

class RedisWatch : public ServiceBase
{
  public:
    RedisWatch(Client& client) : ServiceBase(client) {}
    bool Watch(const std::string& key) {
        std::vector<std::string> key_vec;
        key_vec.push_back(key);
        return Watch(key_vec);
    }
    bool Watch(const std::vector<std::string>& key_vec) {
        std::vector<std::string> send(1,"WATCH");

        for (size_t i = 0; i != key_vec.size(); ++i) {
            send.push_back(key_vec[i]);
        }

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        return true;
    }
    bool UnWatch() {
        std::vector<std::string> send;
        send.push_back("UNWATCH");

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        return true;
    }
};

class RedisTransaction : public ServiceBase
{
  public:
    RedisTransaction(Client& client, const bool autoStart = true)
    : ServiceBase(client),commited_(false),started_(false) {
        if (autoStart) Begin();
    }
    
    bool Begin() {
        if(started_)
            return true;

        commited_= false;
        Reply reply;
        return (started_ = ExecCmd("MULTI", reply));
    }

    bool Commit() {
        if(!started_)
            return false;

        if(commited_)
            return true;

        commited_ = true;
        started_ = false;
        Reply reply;
        return ExecCmd("EXEC", reply) && NIL != reply.Type();
    }

    ~RedisTransaction() {
        Rollback();
    }

  protected:
    bool Rollback() {
        if(started_ && !commited_)
        {
            started_ = false;
            Reply reply;
            return ExecCmd("DISCARD", reply);
        }
        return true; 
    }

    bool ExecCmd(const std::string& cmd,Reply& reply) {
        std::vector<std::string> send;
        send.push_back(cmd);

        if (REDIS_OK != client_.Command(send, reply)) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }
        return true;
    }

  private:
    bool commited_;
    bool started_;
};

/*注释libevent相关实现*/
#if 0
//异步CLIENT暂用libevent封装实现,可酌情修改
typedef boost::function<int32_t (Reply&)> AsyncHandler;

class AsyncClient {
  private:
    AsyncClient(const AsyncClient&);
    AsyncClient& operator = (const AsyncClient&);

  protected:
#ifndef DISABLE_LIB_EVENT
    void InitEvent(void * event) {
        if (NULL == event_)
            event_ = event;
          //  event_ = event_base_new();
    }
    void FreeEvent() {
        if (NULL == event_)
            return;

        DetachEvent();

        //event_base_free((struct event_base *)event_);
        event_ = NULL;
    }
    bool AttachEvent() {
        if (NULL == event_)
            return false;
        return REDIS_OK == redisLibeventAttach(rc_, (struct event_base *)event_);
    }
    void DetachEvent() {
        //redisLibeventDetach(rc_ev.data);
        if(rc_ && rc_->ev.data)
        {
            redisLibeventCleanup(rc_->ev.data);
            rc_->ev.data = NULL;
        }
    }
    void LoopEvent() {
        event_base_dispatch((struct event_base *)event_);
    }
#endif

public:
    // AsyncClient(const std::string& host, int32_t port,
    //             const std::string& pwd,  int32_t db) : event_(NULL) {
    //      Init(host, port, pwd, db);
    //  }

    AsyncClient() : rc_(NULL),event_(NULL),connected_(0)
    {
        //Init(host, port, pwd, db);
    }

    int32_t Init(const std::string& host, int32_t port,
                 const std::string& pwd,  int32_t db, void * event)
    {
        host_   = host;
        port_   = port;
        pwd_    = pwd;
        db_     = db;

        if(NULL == event)
            return -1;

        InitEvent(event);

        return 0;
    }


    bool AsyncCommand(const std::vector<std::string>& cmd_vec, AsyncHandler async_handler) {//async_handler should have a long life period
#ifdef REDIS_DEBUG
        printf("AsyncCommand\n");
#endif
        if (NULL == rc_) {
            if(0 != AsyncConnect())
                return false;
        }
        return AsyncCommandImpl(cmd_vec, async_handler);
    }
    ~AsyncClient() {
        FreeEvent();
        Close();
    }
    const char* ErrMsg() {
        return errmsg_.c_str();
    }
    void * Event()
    {
        return event_;
    }

    void Close()
    {
        if (rc_ != NULL) {
            redisAsyncFree(rc_);
            //redisAsyncDisconnect(rc_);
            rc_ = NULL;
        }
        connected_ = 0;
    }

    void Clear()
    {
        rc_ = NULL; //被动断开后,RC会被REDIS关闭，这里简单设置为空即可
        connected_ = 0;
    }
    void SetConnected()
    {
        connected_ = 2;
    }
    int32_t IsConnected()
    {
        return connected_;
    }
protected:
    bool AsyncCommandImpl(const std::vector<std::string>& cmd_vec, AsyncHandler async_handler) 
    {
#ifdef REDIS_DEBUG
        printf("AsyncCommandImpl\t");
        for (size_t i = 0; i != cmd_vec.size(); ++i) {
            printf("%s ", cmd_vec[i].c_str());
        }
        printf("\n");
#endif
        std::vector<const char*> argv;
        std::vector<size_t> argvlen;
        argv.reserve(cmd_vec.size());
        argvlen.reserve(cmd_vec.size());

        for(size_t i = 0; i != cmd_vec.size(); ++i) {
            argv.push_back(cmd_vec[i].c_str());
            argvlen.push_back(cmd_vec[i].size());
        }

        if(!AttachEvent())
        {
            errmsg_.assign("AttachEvent failed.");
            return false;
        }

        int32_t result = redisAsyncCommandArgv(rc_, &AsyncClient::OnMessage, &async_handler, static_cast<int>(cmd_vec.size()), argv.data(), argvlen.data());
        if (REDIS_ERR == result) {
            errmsg_.assign(rc_->errstr);
            return false;
        }

        LoopEvent();

        return true;
    }

protected:

    static void OnAsyncDisconnect(const struct redisAsyncContext *ac, int status) {//err message lost
        //if (REDIS_OK == status) return;//disconnect by user
#ifdef REDIS_DEBUG
        //disconnect because of error occured, reconnect then
        fprintf(stderr, "[%d]:[%s]\n", ac->err, ac->errstr);
#endif
        //because of timer,simply exit loop for user to reconnect.
        AsyncClient* async_client = (AsyncClient*)ac->data;
        async_client->Clear();//when disconnect ,ac will be free by redis.
        //async_client->DetachEvent();
        //async_client->Close();
        event_base_loopbreak((struct event_base *)async_client->Event());
        //async_client->AsyncConnect();//reconnect
    }
    static void OnAsyncOnconnect(const struct redisAsyncContext *ac, int status) {//err message lost
        AsyncClient* async_client = (AsyncClient*)ac->data;
        if (REDIS_OK == status)
        {
            //connected.
            async_client->SetConnected();
        }
        else
        {
#ifdef REDIS_DEBUG
        //disconnect because of error occured, reconnect then
        fprintf(stderr, "[%d]:[%s]\n", ac->err, ac->errstr);
#endif
           //检测到连接失败会主动清理，这里只简单Clear
           async_client->Clear();
        }
        //because of timer,simply exit loop for user to reconnect.

        event_base_loopbreak((struct event_base *)async_client->Event());
    }

    static void OnMessage(struct redisAsyncContext *ac, void *reply_data, void *privdata) {
#ifdef REDIS_DEBUG
        printf("OnMessage\n");
#endif
        if (NULL == reply_data) {
#ifdef REDIS_DEBUG
            printf("NULL Reply\n");
#endif
            return;
        }

        if (NULL == privdata)
            return;

        Reply reply((redisReply *)reply_data);
        AsyncHandler& async_handler = *(AsyncHandler *)privdata;
        async_handler(reply);
    }

    int32_t OnAuth(Reply& reply) {
#ifdef REDIS_DEBUG
        printf("OnAuth\n");
        reply.Print();
#endif
        DetachEvent();
        event_base_loopbreak((struct event_base *)event_);
        if (reply.Type() == ERROR && !pwd_.empty()) {
            errmsg_.assign("Auth failed").append(rc_->errstr);
            return -1;
        }
        return REDIS_OK;
    }
    int32_t OnSelect(Reply& reply) {
#ifdef REDIS_DEBUG
        printf("OnSelect\n");
        reply.Print();
#endif
        DetachEvent();
        event_base_loopbreak((struct event_base *)event_);
        if (reply.Type() == ERROR) {
            errmsg_.assign("Select failed").append(rc_->errstr);
            return -1;
        }
        return REDIS_OK;
    }
    int32_t OnPing(Reply& reply) {
#ifdef REDIS_DEBUG
        printf("OnPing\n");
        reply.Print();
#endif
        DetachEvent();
        event_base_loopbreak((struct event_base *)event_);
        if (reply.Type() == ERROR)
            return -1;

        return REDIS_OK;
    }

  private:
    int32_t InitConnect() {
        if(Auth() && Ping() && Select())
            return 0;
        return -1;
    }

    bool Auth() {
        if (pwd_.empty())
            return true;

        std::vector<std::string> cmd_vec;
        cmd_vec.push_back("AUTH");
        cmd_vec.push_back(pwd_);
        return AsyncCommand(cmd_vec, boost::bind(&AsyncClient::OnAuth, this, _1));
    }

    bool Ping() {

        std::vector<std::string> cmd_vec;
        cmd_vec.push_back("PING");
        return AsyncCommand(cmd_vec, boost::bind(&AsyncClient::OnPing, this, _1));
    }

    bool Select() {

        std::vector<std::string> cmd_vec;
        cmd_vec.push_back("SELECT");
        cmd_vec.push_back(boost::to_string(static_cast<long long>(db_)));
        return AsyncCommand(cmd_vec, boost::bind(&AsyncClient::OnSelect, this, _1));
    }

    int32_t AsyncConnect()
    {
        Close();//cleanup before reconnect
        rc_ = redisAsyncConnect(host_.c_str(), port_);
        if (NULL == rc_ || REDIS_OK != rc_->err) {
            if(rc_) {
                errmsg_.assign("AsyncConnect failed:").append(rc_->errstr);
                Close();
            }
            else
                errmsg_.assign("redisAsyncContext is null");
            return -1;
        }
        connected_ = 1; //连接中
        rc_->data = this;//pass object to disconnect handler

        //监听连接事件
        if(!AttachEvent())
        {
            errmsg_.assign("AttachEvent failed!");
            Close();
            return -1;
        }

        redisAsyncSetConnectCallback(rc_, &AsyncClient::OnAsyncOnconnect);
        LoopEvent(); //开始监听写事件

        if(2 != connected_)
        {
            //没有监听到事件或者连接出错
            errmsg_.assign("connect failed!");
            Close();
            return -1;
        }
        else
        {
            DetachEvent();//退出循环后干掉监听，以便其他请求打开监听
        }
        //已经建立连接，进行初始化
        if(0 !=InitConnect())
        {
            //这里出错释放的话会CORE。。。暂时不知道如何清理
            //目前这里应该不会失败，如果失败的话调用redis的断链接口
            return -1;
        }

        //设置KEEPALIVE避免死连接
        if (redisEnableKeepAlive(&(rc_->c)) != REDIS_OK) {
            errmsg_.clear();
            errmsg_.append("AsyncConnect keepalive failed:");
            errmsg_.append(std::string(rc_->errstr));
            Close();
            return -1;
        }
        rc_->data = this;//pass object to disconnect handler
        redisAsyncSetDisconnectCallback(rc_, &AsyncClient::OnAsyncDisconnect);//reconect when async disconnect happen

        return 0;
    }

private:
    std::string         host_;
    int32_t             port_;
    std::string         pwd_;
    int32_t             db_;
    AsyncHandler        async_handler_;

    std::string         errmsg_;
    redisAsyncContext  *rc_;

    void               *event_;
    int32_t            connected_; //0：未连接 1：连接中 2：已连接
};

class RedisPublisher : public ServiceBase {
  public:
    RedisPublisher(Client& client) : ServiceBase(client) {}

    int32_t Publish(const std::string& channel, const std::string& msg) {//return the num of the subscriber that receives the message
        std::vector<std::string> cmd_vec;
        cmd_vec.push_back("PUBLISH");
        cmd_vec.push_back(channel);
        cmd_vec.push_back(msg);

        Reply reply;
        int32_t ret = client_.Command(cmd_vec, reply);
        if (REDIS_OK != ret) {
            return -1;
        }
        if (INTEGER != reply.Type()) {
            return -1;
        }
        return reply.Integer();
    }
};

typedef boost::function<int32_t (const std::string&, const std::string&)> SubscribeHandler;

class RedisSubscriber {
  public:
    int32_t AsyncHandlerWrapper(Reply& reply) {
        if (ARRAY == reply.Type() && 3 == reply.Elements().size()) {
            const std::vector<Reply>& elements = reply.Elements();
            if (elements[0].Type() == STRING &&
                elements[1].Type() == STRING &&
                elements[2].Type() == STRING &&
                (elements[0].Str() == "message" || elements[0].Str() == "MESSAGE")) {//process message with the handler_
                return handler_(elements[1].Str(), elements[2].Str());
            }
        }
        DiscardMessage(reply);
        return REDIS_OK;
    }
  protected:
    void DiscardMessage(Reply& reply) {
#ifdef REDIS_DEBUG
        reply.Print();
#endif
    }
  public:
    RedisSubscriber(AsyncClient& client) : client_(client) {

    }
    bool Subscribe(const std::string& channel, SubscribeHandler handler) {
        if (channel.empty()) return false;

        cmd_vec_.clear();
        cmd_vec_.push_back("SUBSCRIBE");
        cmd_vec_.push_back(channel);

        handler_ = handler;

        return SubscribeImpl();
    }
    bool Subscribe(const std::vector<std::string>& channel_vec, SubscribeHandler handler) {
        cmd_vec_.clear();
        cmd_vec_.push_back("SUBSCRIBE");
        for (size_t i = 0; i != channel_vec.size(); ++i) {
            if (!channel_vec[i].empty()) {
                cmd_vec_.push_back(channel_vec[i]);
            }
        }

        if (1 == cmd_vec_.size()) return false;

        handler_ = handler;

        return SubscribeImpl();
    }
    const char* ErrMsg() {
        return client_.ErrMsg();
    }
  protected:
    bool SubscribeImpl() {
        return client_.AsyncCommand(cmd_vec_, boost::bind(&RedisSubscriber::AsyncHandlerWrapper, this, _1));
    }
  private:
    AsyncClient&                client_;
    SubscribeHandler            handler_;
    std::vector<std::string>    cmd_vec_;
};

#endif  /*注释libevent相关封装*/

class RedisLock
{
  public:
    RedisLock(Client& client,const std::string& lock_name)
        : client_(client), lock_name_(lock_name), have_locked_(false)
    {

    }
    bool TryLock(const std::string& lock_info)
    {
        if (have_locked_) {
            return true;
        }
        have_locked_ = client_.Setnx(lock_name_, lock_info);
        return have_locked_;
    }
    bool Unlock()
    {
        if (!have_locked_) {
            return false;
        }
        have_locked_ = !client_.Del(lock_name_);
        return !have_locked_;
    }
    bool ForceUnlock()
    {
        if (have_locked_) {
            return Unlock();
        }
        return client_.Del(lock_name_);
    }
    bool GetLockInfo(std::string& lock_info)
    {
        return client_.Get(lock_name_, lock_info);
    }
    ~RedisLock()
    {

    }
  private:
    RedisString             client_;
    std::string             lock_name_;
    bool                    have_locked_;
};

class RedisScopedLock
{
  public:
    RedisScopedLock(Client& client, const std::string& lock_name)
        : lock_(client, lock_name)
    {

    }
    bool TryLock(const std::string& lock_info)
    {
        return lock_.TryLock(lock_info);
    }
    bool Unlock()
    {
        return lock_.Unlock();
    }
    bool GetLockInfo(std::string& lock_info)
    {
        return lock_.GetLockInfo(lock_info);
    }
    ~RedisScopedLock()
    {
        lock_.Unlock();
    }
  private:
    RedisLock   lock_;
};

class RedisZset : public ServiceBase
{
public:
    RedisZset(Client& client) : ServiceBase(client){}

    bool Zadd(const std::string& key, const std::string& member, const int64_t& score, bool & exist)
    {
        std::vector<std::string> send;
        send.push_back("ZADD");
        send.push_back(key);
        send.push_back(type2str(score) );
        send.push_back(member);

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        exist = (reply.Integer() == 1);
        return true;
    }

    //REDIS 2.4版本以后支持批量zadd
    bool Zadd(const std::string& key, const std::map<std::string, int64_t > & members)
    {
        std::vector<std::string> send;
        send.push_back("ZADD");
        send.push_back(key);

        std::map<std::string, int64_t >::const_iterator it;

        for(it = members.begin(); it != members.end(); it++)
        {
            send.push_back(type2str(it->second));
            send.push_back(it->first);
        }
        

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        return true;
    }

    bool Zrem(const std::string& key, const std::string& member, bool & exist)
    {
        std::vector<std::string> send;
        send.push_back("ZREM");
        send.push_back(key);
        send.push_back(member);

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        if (reply.Type() == ERROR) {
            RecordErrmsg(reply.Str());
            return false;
        }

        exist = (reply.Integer() == 1);
        return true;
    }

    bool Zrem(const std::string& key, const std::vector<std::string> & members)
    {
        std::vector<std::string> send;
        send.push_back("ZREM");
        send.push_back(key);

        std::vector<std::string>::const_iterator it;

        for(it = members.begin(); it != members.end(); it++)
        {
            send.push_back(*it);
        }

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK)
        {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        if (reply.Type() == ERROR)
        {
            RecordErrmsg(reply.Str());
            return false;
        }

        return true;
    }

    bool Zscore(const std::string& key, const std::string& member, std::string & score)
    {
        std::vector<std::string> send;
        send.push_back("ZSCORE");
        send.push_back(key);
        send.push_back(member);

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        if (reply.Type() == ERROR) {
            RecordErrmsg(reply.Str());
            return false;
        }

        score = reply.Str();
        return true;
    }

    bool Zincrby(const std::string& key, const std::string& member, const int64_t& score)
    {
        std::vector<std::string> send;
        send.push_back("ZINCRBY");
        send.push_back(key);
        send.push_back(type2str(score));
        send.push_back(member);

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        return true;
    }

    bool Zrange(const std::string& key, const int32_t start, const int32_t end, 
            std::vector<std::string>& members, std::vector<std::string>& scores)
    {
        std::vector<std::string> send;
        send.push_back("ZRANGE");
        send.push_back(key);
        send.push_back(type2str(start));
        send.push_back(type2str(end));
        send.push_back("WITHSCORES");

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        if (reply.Type() == ERROR) {
            RecordErrmsg(reply.Str());
            return false;
        }

        if (reply.Type() != ARRAY) {
            RecordErrmsg("array result expected");
            return false;
        }

        members.clear();
        scores.clear();
        const std::vector<Reply>& element_vec = reply.Elements();
        for (size_t i = 0; i < element_vec.size(); i += 2) {
            members.push_back(element_vec[i].Str());
            scores.push_back(element_vec[i + 1].Str());
        }

        return true;
    }

    bool ZrangeByScore(const std::string& key, const int64_t& start, const int64_t& end, std::vector<std::string>& members)
    {
        std::vector<std::string> send;
        send.push_back("ZRANGEBYSCORE");
        send.push_back(key);
        send.push_back(type2str(start));
        send.push_back(type2str(end));

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        if (reply.Type() == ERROR) {
            RecordErrmsg(reply.Str());
            return false;
        }

        if (reply.Type() != ARRAY) {
            RecordErrmsg("array result expected");
            return false;
        }

        members.clear();
        const std::vector<Reply>& element_vec = reply.Elements();
        for (size_t i = 0; i != element_vec.size(); ++i) {
            members.push_back(element_vec[i].Str());
        }

        return true;
    }

     bool Zrevrange(const std::string& key, const int32_t start, const int32_t end, 
            std::vector<std::string>& members, std::vector<std::string>& scores)
    {
        std::vector<std::string> send;
        send.push_back("ZREVRANGE");
        send.push_back(key);
        send.push_back(type2str(start));
        send.push_back(type2str(end));
        send.push_back("WITHSCORES");

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        if (reply.Type() == ERROR) {
            RecordErrmsg(reply.Str());
            return false;
        }

        if (reply.Type() != ARRAY) {
            RecordErrmsg("array result expected");
            return false;
        }

        members.clear();
        scores.clear();
        const std::vector<Reply>& element_vec = reply.Elements();
        for (size_t i = 0; i < element_vec.size(); i += 2) {
            members.push_back(element_vec[i].Str());
            scores.push_back(element_vec[i + 1].Str());
        }

        return true;
    }

    bool ZrevrangeByScore(const std::string& key, const int64_t& start, const int64_t& end, std::vector<std::string>& members, const int64_t& offset = 0, const int64_t count = -1)
    {
        std::vector<std::string> send;
        send.push_back("ZREVRANGEBYSCORE");
        send.push_back(key);
        send.push_back(type2str(start));
        send.push_back(type2str(end));

        if (count != -1 || offset > 0)
        {
            send.push_back("LIMIT");
            send.push_back(type2str(offset));
            send.push_back(type2str(count));
        }

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }
        
        if (reply.Type() == ERROR) {
            RecordErrmsg(reply.Str());
            return false;
        }
        

        if (reply.Type() != ARRAY) {
            RecordErrmsg("array result expected");
            return false;
        }

        members.clear();
        const std::vector<Reply>& element_vec = reply.Elements();
        for (size_t i = 0; i != element_vec.size(); ++i) {
            members.push_back(element_vec[i].Str());
        }
        
        return true;
    }

    bool Zrank(const std::string& key, const std::string& member, int64_t& rank)
    {
        std::vector<std::string> send;
        send.push_back("ZRANK");
        send.push_back(key);
        send.push_back(member);

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        if (reply.Type() == ERROR) {
            RecordErrmsg(reply.Str());
            return false;
        }

        if(reply.Type() != NIL)
            rank = reply.Integer();
        else
            rank = -1;
        
        return true;
    }

    bool Zrevrank(const std::string& key, const std::string& member, int64_t& rank)
    {
        std::vector<std::string> send;
        send.push_back("ZREVRANK");
        send.push_back(key);
        send.push_back(member);

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        if (reply.Type() == ERROR) {
            RecordErrmsg(reply.Str());
            return false;
        }

        if(reply.Type() != NIL)
            rank = reply.Integer();
        else
            rank = -1;

        return true;
    }

    bool Zcard(const std::string& key, int64_t& size)
    {
        std::vector<std::string> send;
        send.push_back("ZCARD");
        send.push_back(key);

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        if (reply.Type() == ERROR) {
            RecordErrmsg(reply.Str());
            return false;
        }

        size = reply.Integer();
        return true;
    }
};

class RedisScript : public ServiceBase
{
public:
    RedisScript(Client& client) : ServiceBase(client){}
    ~RedisScript(){}

    bool Eval(const std::string& script, 
                const std::vector<std::string>& keys, 
                const std::vector<std::string>& args,
                Reply & reply)
    {
        std::vector<std::string> send;
        send.push_back("EVAL");
        send.push_back(script);
        send.push_back(type2str(keys.size()));

        for(size_t i = 0; i < keys.size(); i++)
        {
            send.push_back(keys[i]);
        }

        for(size_t i = 0; i < args.size(); i++)
        {
            send.push_back(args[i]);
        }
        
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        if (reply.Type() == ERROR) {
            RecordErrmsg(reply.Str());
            return false;
        }

        return true;
    }

    bool EvalSha(const std::string& sha1, 
                    const std::vector<std::string>& keys, 
                    const std::vector<std::string>& args,
                    Reply & reply)
    {
        std::vector<std::string> send;
        send.push_back("EVALSHA");
        send.push_back(sha1);
        send.push_back(type2str(keys.size()));

        for(size_t i = 0; i < keys.size(); i++)
        {
            send.push_back(keys[i]);
        }

        for(size_t i = 0; i < args.size(); i++)
        {
            send.push_back(args[i]);
        }
        
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        if (reply.Type() == ERROR) {
            RecordErrmsg(reply.Str());
            return false;
        }

        return true;
    }

    bool Load(const std::string& script, std::string& sha1)
    {
        std::vector<std::string> send;
        send.push_back("SCRIPT");
        send.push_back("LOAD");
        send.push_back(script);
        
        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        if (reply.Type() == ERROR) {
            RecordErrmsg(reply.Str());
            return false;
        }

        sha1 = reply.Str();
        return true;
    }

    bool Exists(const std::vector<std::string>& sha1s, std::vector<int32_t>& results)
    {
        std::vector<std::string> send;
        send.push_back("SCRIPT");
        send.push_back("EXISTS");

        for(size_t i = 0; i < sha1s.size(); i++)
        {
            send.push_back(sha1s[i]);
        }
        
        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }

        if (reply.Type() != ARRAY) {
            RecordErrmsg("array result expected");
            return false;
        }

        results.clear();
        const std::vector<Reply>& element_vec = reply.Elements();
        for (size_t i = 0; i != element_vec.size(); ++i) {
            results.push_back(element_vec[i].Integer());
        }

        return true;
    }

    bool Flush()
    {
        std::vector<std::string> send;
        send.push_back("SCRIPT");
        send.push_back("FLUSH");

        Reply reply;
        int32_t ret = client_.Command(send, reply);
        if (ret != REDIS_OK) {
            RecordErrmsg(client_.ErrMsg());
            return false;
        }
        
        if (reply.Type() == ERROR) {
            RecordErrmsg(reply.Str());
            return false;
        }

        return true;
    }
};

} // hiredis
} // demo


#endif
