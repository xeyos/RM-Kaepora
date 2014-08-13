#include "CustomRsa.h"
#include <QDebug>
#include <time.h>
 using namespace std;


CustomRsa::CustomRsa()
{
    srand (time(NULL));
    //Este parámetro se asegura de que solo generamos las keys una vez.
    keys.p = 0;
}

//*******************************************
//*********key generation block**************
//*******************************************
RsaKeys CustomRsa::getKeys(){
    //Si ya hemos generado las keys una vez, devolvemos unas keys inválidas.
    if(keys.p == 0){
        keys.p = getRandomPrime(rand()%(9973-100)+100);
        keys.q = getRandomPrime(rand()%(9973-100)+100);
        keys.n = keys.p*keys.q;
        keys.fi = (keys.p-1)*(keys.q-1);
        keys.e = getRandomRelativePrime(keys.fi);
        keys.d = getInverse(keys.e, keys.fi);
        return keys;
    }
    RsaKeys fake;
    return fake;
}

unsigned int CustomRsa::getRandomPrime(unsigned int a){
    unsigned int i, j;
    for ( i = a; i < 9973; i++ )
    {
        for ( j = 2; j <= i/2; j++ )
        {
            if ( ! ( i % j ) ) break;
        }
        if ( j > i / 2 )
                break;
    }
    return i;
}

unsigned int CustomRsa::getRandomRelativePrime(unsigned int a){
    unsigned r, t;
    do{
        r  = rand()%(9999-100)+100;
    }while(r>=a);
     while ((t = gcd(r, a)) > 1)
         r /= t;
     return r;
}

unsigned int CustomRsa::gcd(unsigned int a, unsigned int b){
    if (b == 0){
        return a;
    }else{
        return gcd(b, a % b);
    }
}

std::pair<unsigned int, std::pair< unsigned int, unsigned int> > extendedEuclid(unsigned int a, unsigned int b) {
    if(a == 0) return make_pair(b, std::make_pair(0, 1));
    std::pair<unsigned int, pair<unsigned int, unsigned int> > p;
    p = extendedEuclid(b % a, a);
    return make_pair(p.first, make_pair(p.second.second - p.second.first*(b/a), p.second.first));
}

unsigned int CustomRsa::getInverse(unsigned int a, unsigned int m) {
    return (extendedEuclid(a,m).second.first + m) % m;
}
//*********************************************************************
//***********Key generation block end ********************************
//*********************************************************************



//*********************************************************************
//***********Cypher block begin ***************************************
//*********************************************************************
void CustomRsa::cypherSymmetricKey(RsaKeys remoteKey, BlowFishKey *symKey){
    //ciframos con la clave pública de nuestro interlocutor
    unsigned int aux = concatenate(symKey->a, symKey->b);
    symKey->cypha = powMod(aux, remoteKey.n, remoteKey.e);
    aux = concatenate(symKey->c, symKey->d);
    symKey->cyphb = powMod(aux, remoteKey.n, remoteKey.e);
    aux = concatenate(symKey->e, symKey->f);
    symKey->cyphc = powMod(aux, remoteKey.n, remoteKey.e);
}

unsigned int CustomRsa::concatenate(unsigned a, unsigned b) {
    unsigned pow = 10;
    while(b >= pow)
        pow *= 10;
    return a * pow + b;
}

unsigned int CustomRsa::cypherSignature(unsigned int sign){
    unsigned int result;
    result = powMod(sign,keys.n, keys.d);
    return result;
}

//*********************************************************************
//***********Cypher block end *****************************************
//*********************************************************************



//*********************************************************************
//***********Decypher block begin *************************************
//*********************************************************************
void CustomRsa::decypherSymmetricKey(RsaKeys remoteKey, BlowFishKey *symkey){
    //Desciframos ahora con nuestra clave privada
    symkey->cypha = powMod(symkey->cypha, keys.n, keys.d);
    symkey->cyphb = powMod(symkey->cyphb, keys.n, keys.d);
    symkey->cyphc = powMod(symkey->cyphc, keys.n, keys.d);
    unconcatenate(symkey);
}

void CustomRsa::unconcatenate(BlowFishKey *symkey){
    symkey->a = symkey->cypha/100;
    symkey->b = symkey->cypha-(symkey->a*100);
    symkey->c = symkey->cyphb/100;
    symkey->d = symkey->cyphb-(symkey->c*100);
    symkey->e = symkey->cyphc/100;
    symkey->f = symkey->cyphc-(symkey->e*100);
}

unsigned int CustomRsa::decypherSignature(RsaKeys remoteKey, unsigned int sign){
    unsigned int result;
    result = powMod(sign, remoteKey.n, remoteKey.e);
    return result;
}

//*********************************************************************
//***********Decypher block end ***************************************
//*********************************************************************


unsigned int CustomRsa::powMod(unsigned int s, unsigned int mod, unsigned int po){
  unsigned long long int base = s%mod;
  unsigned long long int result = 1;
  unsigned int exp = po;
  while (exp > 0) {
    if (exp & 1) result = (result * base) % mod;
    base = (base * base) % mod;
    exp >>= 1;
  }
  return result;
}
