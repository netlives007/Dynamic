#include "key.h"

#include "base58.h"
#include "script/script.h"
#include "uint256.h"
#include "util.h"
#include "utilstrencodings.h"
#include "test/test_dynamic.h"
#include "dht/ed25519.h"


#include <string>
#include <stdint.h>
#include <string.h>
#include <vector>

#include <boost/test/unit_test.hpp>


//ed25519_key_tests

BOOST_FIXTURE_TEST_SUITE(ed25519_key_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(ed25519_key_getlinksharedpubkey_test)
{
    CKeyEd25519 dhtKey1;
    CKeyEd25519 dhtKey2;
    std::vector<unsigned char> vchSharedPubKey1 = GetLinkSharedPubKey(dhtKey1, dhtKey2.GetPubKey());
    std::vector<unsigned char> vchSharedPubKey2 = GetLinkSharedPubKey(dhtKey2, dhtKey1.GetPubKey());
    BOOST_CHECK(vchSharedPubKey1 == vchSharedPubKey2);
    std::cout << "Success: ed25519_key_getlinksharedpubkey_test\n";

}

BOOST_AUTO_TEST_SUITE_END()