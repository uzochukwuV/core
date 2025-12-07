#define NO_UEFI

#include "contract_testing.h"

// Function and Procedure ID constants
constexpr uint32 GET_PRICE_IDX = 1;
constexpr uint32 GET_ALL_PRICES_IDX = 2;
constexpr uint32 UPDATE_PRICE_IDX = 1;  // Procedure ID

class ContractTestingPriceFeed : protected ContractTesting
{
public:
    ContractTestingPriceFeed()
    {
        initEmptySpectrum();
        initEmptyUniverse();
        INIT_CONTRACT(PRICEFEED);
        callSystemProcedure(PRICEFEED_CONTRACT_INDEX, INITIALIZE);
    }

    PRICEFEED::getPrice_output getPrice(uint8 token_id)
    {
        PRICEFEED::getPrice_input input{ token_id };
        PRICEFEED::getPrice_output output;
        callFunction(PRICEFEED_CONTRACT_INDEX, GET_PRICE_IDX, input, output);
        return output;
    }

    PRICEFEED::getAllPrices_output getAllPrices()
    {
        PRICEFEED::getAllPrices_input input{};
        PRICEFEED::getAllPrices_output output;
        callFunction(PRICEFEED_CONTRACT_INDEX, GET_ALL_PRICES_IDX, input, output);
        return output;
    }

    PRICEFEED::updatePrice_output updatePrice(uint8 token_id, uint64 new_price, const id& caller)
    {
        // Ensure user has energy
        increaseEnergy(caller, 1000000);
        PRICEFEED::updatePrice_input input{ token_id, new_price };
        PRICEFEED::updatePrice_output output;
        invokeUserProcedure(PRICEFEED_CONTRACT_INDEX, UPDATE_PRICE_IDX, input, output, caller, 0);
        return output;
    }
};

TEST(ContractPriceFeed, Initialization)
{
    ContractTestingPriceFeed pricefeed;
    auto prices = pricefeed.getAllPrices();
    EXPECT_GT(prices.qubic, 0);
    EXPECT_GT(prices.btc, 0);
    EXPECT_GT(prices.bnb, 0);
    EXPECT_GT(prices.usdt, 0);
}

TEST(ContractPriceFeed, GetPrice_Qubic)
{
    ContractTestingPriceFeed pricefeed;
    auto price = pricefeed.getPrice(PRICEFEED_QUBIC);
    EXPECT_GT(price.current_price, 0);
    EXPECT_TRUE(price.is_valid);
}

TEST(ContractPriceFeed, GetPrice_Bitcoin)
{
    ContractTestingPriceFeed pricefeed;
    auto price = pricefeed.getPrice(PRICEFEED_BTC);
    EXPECT_GT(price.current_price, 0);
    EXPECT_TRUE(price.is_valid);
}

TEST(ContractPriceFeed, GetPrice_BNB)
{
    ContractTestingPriceFeed pricefeed;
    auto price = pricefeed.getPrice(PRICEFEED_BNB);
    EXPECT_GT(price.current_price, 0);
    EXPECT_TRUE(price.is_valid);
}

TEST(ContractPriceFeed, GetPrice_USDT)
{
    ContractTestingPriceFeed pricefeed;
    auto price = pricefeed.getPrice(PRICEFEED_USDT);
    EXPECT_GT(price.current_price, 0);
    EXPECT_TRUE(price.is_valid);
}

TEST(ContractPriceFeed, GetPrice_InvalidToken)
{
    ContractTestingPriceFeed pricefeed;
    auto price = pricefeed.getPrice(10);
    EXPECT_FALSE(price.is_valid);
}

TEST(ContractPriceFeed, UpdatePrice_Valid)
{
    ContractTestingPriceFeed pricefeed;
    id oracle_address(0x11111111, 0x22222222, 0x33333333, 0x44444444);
    auto output = pricefeed.updatePrice(PRICEFEED_BTC, 50000000000ULL, oracle_address);
    EXPECT_TRUE(output.success);
}

TEST(ContractPriceFeed, UpdatePrice_Zero)
{
    ContractTestingPriceFeed pricefeed;
    id oracle_address(0xaaaabbbb, 0xccccdddd, 0xeeeeeeee, 0xffffffff);
    auto output = pricefeed.updatePrice(PRICEFEED_QUBIC, 0, oracle_address);
    EXPECT_FALSE(output.success);
}

TEST(ContractPriceFeed, UpdatePrice_InvalidToken)
{
    ContractTestingPriceFeed pricefeed;
    id oracle_address(0x12345678, 0x87654321, 0xabcdef00, 0xfedcba00);
    auto output = pricefeed.updatePrice(10, 1000000ULL, oracle_address);
    EXPECT_FALSE(output.success);
}

TEST(ContractPriceFeed, UpdatePrice_VeryLargePrice)
{
    ContractTestingPriceFeed pricefeed;
    id oracle_address(0xfedcba98, 0x76543210, 0x13579ace, 0x02468bdf);
    auto output = pricefeed.updatePrice(PRICEFEED_BTC, 0xFFFFFFFFFFFFFFFFULL, oracle_address);
    EXPECT_TRUE(output.success);
}

TEST(ContractPriceFeed, UpdatePrice_SmallPrice)
{
    ContractTestingPriceFeed pricefeed;
    id oracle_address(0x10203040, 0x50607080, 0x90a0b0c0, 0xd0e0f000);
    auto output = pricefeed.updatePrice(PRICEFEED_USDT, 1, oracle_address);
    EXPECT_TRUE(output.success);
}

TEST(ContractPriceFeed, UpdatePrice_Consecutive)
{
    ContractTestingPriceFeed pricefeed;
    id oracle_address(0xa1b1c1d1, 0xe1f1a2b2, 0xc2d2e2f2, 0xa3b3c3d3);
    auto output1 = pricefeed.updatePrice(PRICEFEED_BTC, 45000000000ULL, oracle_address);
    EXPECT_TRUE(output1.success);
    auto output2 = pricefeed.updatePrice(PRICEFEED_BTC, 48000000000ULL, oracle_address);
    EXPECT_TRUE(output2.success);
}

TEST(ContractPriceFeed, GetAllPrices_AllSet)
{
    ContractTestingPriceFeed pricefeed;
    auto prices = pricefeed.getAllPrices();
    EXPECT_GT(prices.qubic, 0);
    EXPECT_GT(prices.btc, 0);
    EXPECT_GT(prices.bnb, 0);
    EXPECT_GT(prices.usdt, 0);
}

TEST(ContractPriceFeed, UpdatePrice_MultipleTokens)
{
    ContractTestingPriceFeed pricefeed;
    id oracle(0x11223344, 0x55667788, 0x99aabbcc, 0xddeeff00);
    auto out1 = pricefeed.updatePrice(PRICEFEED_QUBIC, 2500000ULL, oracle);
    auto out2 = pricefeed.updatePrice(PRICEFEED_BTC, 52000000000ULL, oracle);
    auto out3 = pricefeed.updatePrice(PRICEFEED_BNB, 700000000ULL, oracle);
    auto out4 = pricefeed.updatePrice(PRICEFEED_USDT, 1000000ULL, oracle);
    EXPECT_TRUE(out1.success);
    EXPECT_TRUE(out2.success);
    EXPECT_TRUE(out3.success);
    EXPECT_TRUE(out4.success);
}

TEST(ContractPriceFeed, PriceIndependence)
{
    ContractTestingPriceFeed pricefeed;
    id oracle(0xf0f1f2f3, 0xf4f5f6f7, 0xf8f9fafb, 0xfcfdfeff);
    auto update_btc = pricefeed.updatePrice(PRICEFEED_BTC, 60000000000ULL, oracle);
    EXPECT_TRUE(update_btc.success);
    auto update_bnb = pricefeed.updatePrice(PRICEFEED_BNB, 500000000ULL, oracle);
    EXPECT_TRUE(update_bnb.success);
    auto btc_price = pricefeed.getPrice(PRICEFEED_BTC);
    auto bnb_price = pricefeed.getPrice(PRICEFEED_BNB);
    EXPECT_EQ(btc_price.current_price, 60000000000ULL);
    EXPECT_EQ(bnb_price.current_price, 500000000ULL);
    EXPECT_NE(btc_price.current_price, bnb_price.current_price);
}
