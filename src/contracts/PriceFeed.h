using namespace QPI;

// Token IDs for price feeds (modular design)
enum PriceFeedTokenId {
    PRICEFEED_QUBIC = 0,
    PRICEFEED_BTC = 1,
    PRICEFEED_BNB = 2,
    PRICEFEED_USDT = 3
};

// Price precision: 1e8 (8 decimal places)
constexpr uint64 PRICEFEED_PRICE_PRECISION = 100000000ULL;

// Initial prices (for testing)
constexpr uint64 PRICEFEED_INITIAL_PRICE_QUBIC = 10000000ULL;    // 0.1 USD
constexpr uint64 PRICEFEED_INITIAL_PRICE_BTC = 4500000000000ULL; // 45000 USD
constexpr uint64 PRICEFEED_INITIAL_PRICE_BNB = 60000000000ULL;   // 600 USD
constexpr uint64 PRICEFEED_INITIAL_PRICE_USDT = 100000000ULL;    // 1 USD

struct PRICEFEED2 {};

struct PRICEFEED : public ContractBase {
public:
    // Storage for prices and metadata
    uint64 prices[4];           // Current prices for 4 tokens
    uint64 last_update_tick[4]; // Last update tick for each token

    // ==================== INPUT/OUTPUT STRUCTS ====================

    struct getPrice_input {
        uint8 token_id;
    };

    struct getPrice_output {
        uint64 current_price;     // Price with 1e8 precision
        uint64 last_update_tick;
        bool is_valid;
    };

    struct getAllPrices_input {};

    struct getAllPrices_output {
        uint64 qubic;
        uint64 btc;
        uint64 bnb;
        uint64 usdt;
    };

    struct updatePrice_input {
        uint8 token_id;
        uint64 new_price;
    };

    struct updatePrice_output {
        bool success;
    };

    // ==================== PUBLIC FUNCTIONS (READ-ONLY) ====================

    PUBLIC_FUNCTION(getPrice) {
        if (input.token_id >= 4) {
            output.is_valid = false;
            output.current_price = 0;
            output.last_update_tick = 0;
            return;
        }

        output.current_price = state.prices[input.token_id];
        output.last_update_tick = state.last_update_tick[input.token_id];
        output.is_valid = true;
    }

    PUBLIC_FUNCTION(getAllPrices) {
        output.qubic = state.prices[PRICEFEED_QUBIC];
        output.btc = state.prices[PRICEFEED_BTC];
        output.bnb = state.prices[PRICEFEED_BNB];
        output.usdt = state.prices[PRICEFEED_USDT];
    }

    // ==================== PUBLIC PROCEDURES (STATE-MODIFYING) ====================

    PUBLIC_PROCEDURE(updatePrice) {
        // Validate token ID
        if (input.token_id >= 4) {
            output.success = false;
            return;
        }

        // Validate price (must be greater than 0)
        if (input.new_price == 0) {
            output.success = false;
            return;
        }

        // Update price storage
        state.prices[input.token_id] = input.new_price;
        state.last_update_tick[input.token_id] = qpi.tick();

        output.success = true;
    }

    // ==================== SYSTEM PROCEDURES ====================

    INITIALIZE() {
        // Initialize all prices with default values
        state.prices[PRICEFEED_QUBIC] = PRICEFEED_INITIAL_PRICE_QUBIC;
        state.prices[PRICEFEED_BTC] = PRICEFEED_INITIAL_PRICE_BTC;
        state.prices[PRICEFEED_BNB] = PRICEFEED_INITIAL_PRICE_BNB;
        state.prices[PRICEFEED_USDT] = PRICEFEED_INITIAL_PRICE_USDT;

        // Set initial update ticks
        for (uint8 i = 0; i < 4; i++) {
            state.last_update_tick[i] = 0;
        }
    }

    // ==================== REGISTRATION ====================

    REGISTER_USER_FUNCTIONS_AND_PROCEDURES() {
        REGISTER_USER_FUNCTION(getPrice, 1);
        REGISTER_USER_FUNCTION(getAllPrices, 2);
        REGISTER_USER_PROCEDURE(updatePrice, 1);
    }
};
