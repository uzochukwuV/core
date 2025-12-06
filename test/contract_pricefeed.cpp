#define NO_UEFI

#include "contract_testing.h"

#define PRINT_TEST_INFO 0

static const id PRICEFEED_CONTRACT_ID(PRICEFEED_CONTRACT_INDEX, 0, 0, 0);

// ==================== TEST CLASS ====================

class PriceFeedChecker : public PRICEFEED
{
public:
    void checkPriceValidity(uint8 token_id, uint64 expected_price)
    {
        EXPECT_LT(token_id, 4);
        EXPECT_EQ(state.prices[token_id], expected_price);
    }

    void checkAllPricesInitialized()
    {
        EXPECT_GT(state.prices[PRICEFEED_QUBIC], 0);
        EXPECT_GT(state.prices[PRICEFEED_BTC], 0);
        EXPECT_GT(state.prices[PRICEFEED_BNB], 0);
        EXPECT_GT(state.prices[PRICEFEED_USDT], 0);
    }
};

class ContractTestingPriceFeed : protected ContractTesting
{
public:
    ContractTestingPriceFeed()
    {
        INIT_CONTRACT(PRICEFEED);
        initEmptySpectrum();
    }

    // Helper to call getPrice function
    void callGetPrice(uint8 token_id, PRICEFEED::getPrice_output& output)
    {
        PRICEFEED::getPrice_input input{ token_id };
        callFunction(PRICEFEED_CONTRACT_INDEX, 1, input, output, true, true);
    }

    // Helper to call getAllPrices function
    void callGetAllPrices(PRICEFEED::getAllPrices_output& output)
    {
        PRICEFEED::getAllPrices_input input{};
        callFunction(PRICEFEED_CONTRACT_INDEX, 2, input, output, true, true);
    }

    // Helper to call getOracleAddress function
    void callGetOracleAddress(PRICEFEED::getOracleAddress_output& output)
    {
        PRICEFEED::getOracleAddress_input input{};
        callFunction(PRICEFEED_CONTRACT_INDEX, 3, input, output, true, true);
    }

    // Helper to call updatePrice procedure
    void callUpdatePrice(uint8 token_id, uint64 new_price, const id& caller, PRICEFEED::updatePrice_output& output)
    {
        PRICEFEED::updatePrice_input input{ token_id, new_price };
        invokeUserProcedure(PRICEFEED_CONTRACT_INDEX, 1, input, output, caller, 0, true, true);
    }

    // Helper to call setOracleAddress procedure
    void callSetOracleAddress(const id& new_oracle, const id& caller, PRICEFEED::setOracleAddress_output& output)
    {
        PRICEFEED::setOracleAddress_input input{ new_oracle };
        invokeUserProcedure(PRICEFEED_CONTRACT_INDEX, 2, input, output, caller, 0, true, true);
    }
};

// ==================== TESTS ====================

TEST_F(ContractTestingPriceFeed, Initialization)
{
    // Call INITIALIZE system procedure
    callSystemProcedure(PRICEFEED_CONTRACT_INDEX, INITIALIZE, true);

    // Check that prices are initialized
    PRICEFEED::getPrice_output qubic_output;
    callGetPrice(PRICEFEED_QUBIC, qubic_output);
    EXPECT_TRUE(qubic_output.is_valid);
    EXPECT_EQ(qubic_output.current_price, 10000000ULL); // 0.1 USD

    PRICEFEED::getPrice_output btc_output;
    callGetPrice(PRICEFEED_BTC, btc_output);
    EXPECT_TRUE(btc_output.is_valid);
    EXPECT_EQ(btc_output.current_price, 4500000000000ULL); // 45000 USD

    PRICEFEED::getPrice_output bnb_output;
    callGetPrice(PRICEFEED_BNB, bnb_output);
    EXPECT_TRUE(bnb_output.is_valid);
    EXPECT_EQ(bnb_output.current_price, 60000000000ULL); // 600 USD

    PRICEFEED::getPrice_output usdt_output;
    callGetPrice(PRICEFEED_USDT, usdt_output);
    EXPECT_TRUE(usdt_output.is_valid);
    EXPECT_EQ(usdt_output.current_price, 100000000ULL); // 1 USD
}

TEST_F(ContractTestingPriceFeed, GetPrice_ValidToken)
{
    callSystemProcedure(PRICEFEED_CONTRACT_INDEX, INITIALIZE, true);

    PRICEFEED::getPrice_output output;
    callGetPrice(PRICEFEED_BTC, output);

    EXPECT_TRUE(output.is_valid);
    EXPECT_EQ(output.current_price, 4500000000000ULL);
    EXPECT_EQ(output.last_update_tick, 0);
}

TEST_F(ContractTestingPriceFeed, GetPrice_InvalidToken)
{
    callSystemProcedure(PRICEFEED_CONTRACT_INDEX, INITIALIZE, true);

    PRICEFEED::getPrice_output output;
    callGetPrice(99, output); // Invalid token ID

    EXPECT_FALSE(output.is_valid);
    EXPECT_EQ(output.current_price, 0);
    EXPECT_EQ(output.last_update_tick, 0);
}

TEST_F(ContractTestingPriceFeed, GetAllPrices)
{
    callSystemProcedure(PRICEFEED_CONTRACT_INDEX, INITIALIZE, true);

    PRICEFEED::getAllPrices_output output;
    callGetAllPrices(output);

    EXPECT_EQ(output.qubic, 10000000ULL);
    EXPECT_EQ(output.btc, 4500000000000ULL);
    EXPECT_EQ(output.bnb, 60000000000ULL);
    EXPECT_EQ(output.usdt, 100000000ULL);
}

TEST_F(ContractTestingPriceFeed, UpdatePrice_ValidToken)
{
    callSystemProcedure(PRICEFEED_CONTRACT_INDEX, INITIALIZE, true);

    id test_caller(0x12345678, 0x87654321, 0xabcdef00, 0xfedcba00);
    PRICEFEED::updatePrice_output output;

    // Update BTC price
    uint64 new_btc_price = 5000000000000ULL; // 50000 USD
    callUpdatePrice(PRICEFEED_BTC, new_btc_price, test_caller, output);

    EXPECT_TRUE(output.success);

    // Verify price was updated
    PRICEFEED::getPrice_output get_output;
    callGetPrice(PRICEFEED_BTC, get_output);
    EXPECT_EQ(get_output.current_price, new_btc_price);
    EXPECT_GT(get_output.last_update_tick, 0);
}

TEST_F(ContractTestingPriceFeed, UpdatePrice_MultipleTokens)
{
    callSystemProcedure(PRICEFEED_CONTRACT_INDEX, INITIALIZE, true);

    id test_caller(0x11111111, 0x22222222, 0x33333333, 0x44444444);
    PRICEFEED::updatePrice_output output;

    // Update all token prices
    callUpdatePrice(PRICEFEED_QUBIC, 20000000ULL, test_caller, output);
    EXPECT_TRUE(output.success);

    callUpdatePrice(PRICEFEED_BTC, 5500000000000ULL, test_caller, output);
    EXPECT_TRUE(output.success);

    callUpdatePrice(PRICEFEED_BNB, 75000000000ULL, test_caller, output);
    EXPECT_TRUE(output.success);

    callUpdatePrice(PRICEFEED_USDT, 101000000ULL, test_caller, output);
    EXPECT_TRUE(output.success);

    // Verify all prices
    PRICEFEED::getAllPrices_output all_output;
    callGetAllPrices(all_output);

    EXPECT_EQ(all_output.qubic, 20000000ULL);
    EXPECT_EQ(all_output.btc, 5500000000000ULL);
    EXPECT_EQ(all_output.bnb, 75000000000ULL);
    EXPECT_EQ(all_output.usdt, 101000000ULL);
}

TEST_F(ContractTestingPriceFeed, UpdatePrice_InvalidToken)
{
    callSystemProcedure(PRICEFEED_CONTRACT_INDEX, INITIALIZE, true);

    id test_caller(0xaaaabbbb, 0xccccdddd, 0xeeeeeeee, 0xffffffff);
    PRICEFEED::updatePrice_output output;

    // Try to update invalid token
    callUpdatePrice(99, 1000000ULL, test_caller, output);

    EXPECT_FALSE(output.success);
    EXPECT_EQ(output.error_code, 1);
}

TEST_F(ContractTestingPriceFeed, UpdatePrice_ZeroPrice)
{
    callSystemProcedure(PRICEFEED_CONTRACT_INDEX, INITIALIZE, true);

    id test_caller(0x12341234, 0x56785678, 0xabcdabcd, 0xefefefef);
    PRICEFEED::updatePrice_output output;

    // Try to update with zero price
    callUpdatePrice(PRICEFEED_BTC, 0, test_caller, output);

    EXPECT_FALSE(output.success);
    EXPECT_EQ(output.error_code, 2);

    // Verify price wasn't changed
    PRICEFEED::getPrice_output get_output;
    callGetPrice(PRICEFEED_BTC, get_output);
    EXPECT_EQ(get_output.current_price, 4500000000000ULL); // Still original price
}

TEST_F(ContractTestingPriceFeed, UpdatePrice_LargeValues)
{
    callSystemProcedure(PRICEFEED_CONTRACT_INDEX, INITIALIZE, true);

    id test_caller(0xfedcbaaa, 0x98765432, 0x13579bdf, 0xacefaced);
    PRICEFEED::updatePrice_output output;

    // Update with very large price (e.g., 1 million USD per unit)
    uint64 very_large_price = 100000000000000ULL; // 1,000,000 USD
    callUpdatePrice(PRICEFEED_BTC, very_large_price, test_caller, output);

    EXPECT_TRUE(output.success);

    // Verify price was set correctly
    PRICEFEED::getPrice_output get_output;
    callGetPrice(PRICEFEED_BTC, get_output);
    EXPECT_EQ(get_output.current_price, very_large_price);
}

TEST_F(ContractTestingPriceFeed, UpdatePrice_SmallValues)
{
    callSystemProcedure(PRICEFEED_CONTRACT_INDEX, INITIALIZE, true);

    id test_caller(0x01020304, 0x05060708, 0x090a0b0c, 0x0d0e0f10);
    PRICEFEED::updatePrice_output output;

    // Update with very small price (e.g., 0.00000001 USD)
    uint64 very_small_price = 1ULL;
    callUpdatePrice(PRICEFEED_QUBIC, very_small_price, test_caller, output);

    EXPECT_TRUE(output.success);

    // Verify price was set correctly
    PRICEFEED::getPrice_output get_output;
    callGetPrice(PRICEFEED_QUBIC, get_output);
    EXPECT_EQ(get_output.current_price, very_small_price);
}

TEST_F(ContractTestingPriceFeed, PriceUpdateTimestamp)
{
    callSystemProcedure(PRICEFEED_CONTRACT_INDEX, INITIALIZE, true);

    // Verify initial timestamps are 0
    PRICEFEED::getPrice_output output1;
    callGetPrice(PRICEFEED_QUBIC, output1);
    EXPECT_EQ(output1.last_update_tick, 0);

    // Update price
    id test_caller(0xaabbccdd, 0x11223344, 0x55667788, 0x99aabbcc);
    PRICEFEED::updatePrice_output update_output;
    callUpdatePrice(PRICEFEED_QUBIC, 15000000ULL, test_caller, update_output);
    EXPECT_TRUE(update_output.success);

    // Check that timestamp was updated
    PRICEFEED::getPrice_output output2;
    callGetPrice(PRICEFEED_QUBIC, output2);
    EXPECT_GT(output2.last_update_tick, 0);
}

TEST_F(ContractTestingPriceFeed, ConsecutiveUpdates)
{
    callSystemProcedure(PRICEFEED_CONTRACT_INDEX, INITIALIZE, true);

    id test_caller(0x13579ace, 0x02468bdf, 0xfedcba98, 0x76543210);

    // Update price multiple times
    for (int i = 0; i < 10; i++)
    {
        uint64 new_price = 5000000000000ULL + (i * 100000000000ULL);
        PRICEFEED::updatePrice_output output;
        callUpdatePrice(PRICEFEED_BTC, new_price, test_caller, output);
        EXPECT_TRUE(output.success);

        // Verify the update took effect
        PRICEFEED::getPrice_output get_output;
        callGetPrice(PRICEFEED_BTC, get_output);
        EXPECT_EQ(get_output.current_price, new_price);
    }
}

TEST_F(ContractTestingPriceFeed, AllTokensIndependent)
{
    callSystemProcedure(PRICEFEED_CONTRACT_INDEX, INITIALIZE, true);

    id test_caller(0xfedcba98, 0x76543210, 0x13579ace, 0x02468bdf);
    PRICEFEED::updatePrice_output output;

    // Update each token independently and verify others don't change
    uint64 original_prices[4];
    PRICEFEED::getAllPrices_output all_output;
    callGetAllPrices(all_output);
    original_prices[0] = all_output.qubic;
    original_prices[1] = all_output.btc;
    original_prices[2] = all_output.bnb;
    original_prices[3] = all_output.usdt;

    // Update BTC only
    callUpdatePrice(PRICEFEED_BTC, 6000000000000ULL, test_caller, output);
    EXPECT_TRUE(output.success);

    callGetAllPrices(all_output);
    EXPECT_EQ(all_output.qubic, original_prices[0]);
    EXPECT_EQ(all_output.btc, 6000000000000ULL);
    EXPECT_EQ(all_output.bnb, original_prices[2]);
    EXPECT_EQ(all_output.usdt, original_prices[3]);

    // Update USDT only
    callUpdatePrice(PRICEFEED_USDT, 105000000ULL, test_caller, output);
    EXPECT_TRUE(output.success);

    callGetAllPrices(all_output);
    EXPECT_EQ(all_output.qubic, original_prices[0]);
    EXPECT_EQ(all_output.btc, 6000000000000ULL);
    EXPECT_EQ(all_output.bnb, original_prices[2]);
    EXPECT_EQ(all_output.usdt, 105000000ULL);
}
