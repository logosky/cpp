#pragma once

#include <string>

#include <openssl/rsa.h>
#include <openssl/aes.h>
#include <openssl/bio.h>
#include <openssl/pem.h>

class EncryptHelper
{
public:

    // 输入任意长度字符串, 会自动补齐0凑足16字节长度的倍数, 输出密文长度一定是16字节的整数倍
    static int encrypt_aes(const std::string & key, const std::string & data, std::string & output)
    {
        AES_KEY aes_key;
        if (AES_set_encrypt_key((const unsigned char*)key.c_str(), key.length() * 8, &aes_key) < 0)
        {
            return -1;
        }

        std::vector<unsigned char> vec_ret;
        
        std::string data_bak = data;
        unsigned int data_length = data_bak.length();
        int padding = 0;
        if (data_bak.length() % AES_BLOCK_SIZE > 0)
        {
            padding = AES_BLOCK_SIZE - data_bak.length() % AES_BLOCK_SIZE;
        }
        data_length += padding;
        while (padding > 0)
        {
            data_bak += '\0';
            padding--;
        }
        vec_ret.resize(data_bak.size());

        for (unsigned int i = 0; i < data_length / AES_BLOCK_SIZE; i++)
        {
            std::string str16 = data_bak.substr(i*AES_BLOCK_SIZE, AES_BLOCK_SIZE);
            unsigned char * out = (unsigned char *)&vec_ret[0] + i * AES_BLOCK_SIZE;
            AES_encrypt((const unsigned char*)str16.c_str(), out, &aes_key);
        }
        
        output.assign((char *)&vec_ret[0], vec_ret.size());

        return 0;
    }

    // 密文一定是16字节的整数倍
    static int decrypt_aes(const std::string& key, const std::string& strData, std::string & output)
    {
        if (strData.empty() || strData.length() % 16 > 0)
        {
            return -1;
        }

        AES_KEY aes_key;
        if (AES_set_decrypt_key((const unsigned char*)key.c_str(), key.length() * 8, &aes_key) < 0)
        {
            return -1;
        }

        std::vector<unsigned char> vec_ret;
        vec_ret.resize(strData.length());

        for (unsigned int i = 0; i < strData.length() / AES_BLOCK_SIZE; i++)
        {
            std::string str16 = strData.substr(i*AES_BLOCK_SIZE, AES_BLOCK_SIZE);
            unsigned char * out = (unsigned char *)&vec_ret[0] + i * AES_BLOCK_SIZE;
            AES_decrypt((const unsigned char*)str16.c_str(), out, &aes_key);
        }
        
        output.assign((char *)&vec_ret[0], vec_ret.size());
        
        return 0;
    }

    // rsa公钥加密
    static std::string rsa_encrypt_with_pub(const std::string & pub_key, const std::string & content)
    {
        BIO *bio = NULL;
        RSA *publicRsa = NULL;

        if ((bio = BIO_new_mem_buf((void *)pub_key.c_str(), -1)) == NULL)
        {
            LOG("BIO_new_mem_buf publicKey error");
            return "";
        }

        if ((publicRsa = PEM_read_bio_RSA_PUBKEY(bio, NULL, NULL, NULL)) == NULL)
        {
            LOG("PEM_read_bio_RSA_PUBKEY error");
            return "";
        }
        BIO_free_all(bio);

        int rsa_len = RSA_size(publicRsa);

        unsigned char *encryptMsg = (unsigned char *)malloc(std::max<int>(11+content.size(), rsa_len));

        std::string encrypt_ret;
        int ret = RSA_public_encrypt(content.size(), (uint8_t *)content.c_str(), encryptMsg, publicRsa, RSA_PKCS1_PADDING);
        if(ret < 0)
        {
            LOG("RSA_public_encrypt error");
        }
        else
        {
            encrypt_ret.assign((char *)encryptMsg, ret);
        }
        
        RSA_free(publicRsa);
        free(encryptMsg);
        return encrypt_ret;
    }

    // rsa私钥解密
    static std::string rsa_decrypt_with_private(const std::string & private_key, const std::string & encrypt_content)
    {
        RSA *privateRsa = NULL;
        BIO *bio = NULL;

        if ((bio = BIO_new_mem_buf((void *)private_key.c_str(), -1)) == NULL)
        {
            LOG("BIO_new_mem_buf privateKey error ");
            return "";
        }
        
        // OpenSSL_add_all_algorithms();
        // 密钥有经过口令加密需要这个函数
        if ((privateRsa = PEM_read_bio_RSAPrivateKey(bio, NULL, NULL, (char *)NULL)) == NULL)
        {
            LOG("PEM_read_RSAPrivateKey error ");
            return "";
        }
        BIO_free_all(bio);

        int rsa_len = RSA_size(privateRsa);
        unsigned char *decryptMsg = (unsigned char *)malloc(std::max<int>(rsa_len, encrypt_content.size()));

        int mun = RSA_private_decrypt(rsa_len, (uint8_t*)encrypt_content.c_str(), decryptMsg, privateRsa, RSA_PKCS1_PADDING);

        std::string encrypt_ret;
        if (mun < 0)
        {
            LOG("RSA_private_decrypt error");
        }
        else
        {
            encrypt_ret.assign((char *)decryptMsg, mun);
        }
        
        delete decryptMsg;
        decryptMsg = NULL;

        RSA_free(privateRsa);

        return encrypt_ret;
    }
};


