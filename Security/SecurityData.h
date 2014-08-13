#ifndef SECURITYDATA_H
#define SECURITYDATA_H

struct BlowFishKey {
    unsigned int a,b,c,d,e,f;
    unsigned int cypha, cyphb, cyphc;
};

struct RsaKeys {
    unsigned int p,q,n,fi,e,d;
};

#endif // SECURITYDATA_H
