#ifndef FIRO_PRIMITIVES_H
#define FIRO_PRIMITIVES_H

#include "libspark/coin.h"
#include "serialize.h"
#include "../uint256.h"

struct CSparkMintMeta
{
    int nHeight;
    int nId;
    bool isUsed;
    uint256 txid;
    uint64_t i; // diversifier
    std::vector<unsigned char> d; // encrypted diversifier
    uint64_t v; // value
    Scalar k; // nonce
    std::string memo; // memo
    std::vector<unsigned char> serial_context;
    mutable boost::optional<uint256> nonceHash;

    uint256 GetNonceHash() const;

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(nHeight);
        READWRITE(nId);
        READWRITE(isUsed);
        READWRITE(txid);
        READWRITE(i);
        READWRITE(d);
        READWRITE(v);
        READWRITE(k);
        READWRITE(memo);
        READWRITE(serial_context);
    };
};


namespace primitives {
    uint256 GetNonceHash(const secp_primitives::Scalar& nonce);
    uint256 GetLTagHash(const secp_primitives::GroupElement& tag);
    uint256 GetSparkCoinHash(const spark::Coin& coin);
}

namespace spark {
// Custom hash for the spark coin. norte. THIS IS NOT SECURE HASH FUNCTION
struct CoinHash {
    std::size_t operator()(const spark::Coin& coin) const noexcept;
};

// Custom hash for the linking tag. THIS IS NOT SECURE HASH FUNCTION
struct CLTagHash {
    std::size_t operator()(const secp_primitives::GroupElement& tag) const noexcept;
};

struct CMintedCoinInfo {
    int coinGroupId;
    int nHeight;

    static CMintedCoinInfo make(int coinGroupId, int nHeight);
};

}


#endif //FIRO_PRIMITIVES_H
