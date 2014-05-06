#ifndef CUSTOMRSA_H
#define CUSTOMRSA_H
#include "SecurityData.h"

class CustomRsa
{
public:
    CustomRsa();
    RsaKeys getKeys();
    void cypherSymmetricKey(RsaKeys remoteKey, BlowFishKey *symKey);
    void decypherSymmetricKey(BlowFishKey *symkey);
    unsigned int cypherSignature(unsigned int sign);
    unsigned int decypherSignature(RsaKeys remoteKey, unsigned int sign);

private:
    unsigned int gcd(unsigned int a, unsigned int b);
    unsigned int getRandomRelativePrime(unsigned int a);
    unsigned int getRandomPrime(unsigned int a);
    unsigned int getInverse(unsigned int a, unsigned int mod);
    unsigned int concatenate(unsigned a, unsigned b);
    unsigned int powMod(unsigned int s, unsigned int mod, unsigned int po);
    void unconcatenate(BlowFishKey *symkey);

private:
    RsaKeys keys;
};

#endif // CUSTOMRSA_H
