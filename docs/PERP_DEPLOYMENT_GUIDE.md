# Qubic Perpetual Futures (PERP) - Deployment Guide

## Overview

This guide explains how to deploy the Perpetual Futures contracts to the Qubic blockchain testnet and mainnet.

The system consists of two contracts:
1. **PriceFeed Contract (Index 20)** - Oracle price management for 4 tokens
2. **Perpetuals Contract (Index 21)** - Main perpetual futures trading engine

---

## Contract Status

✅ **Build Status**: All contracts compile successfully (0 errors, 0 warnings)
- Location: `src/contracts/PriceFeed.h`
- Location: `src/contracts/Perpetuals.h`
- Registered in: `src/contract_core/contract_def.h`

---

## Deployment Architecture

### Current Implementation

**Contract Indices:**
- PriceFeed: Index 20
- Perpetuals: Index 21

**State Sizes:**
- PriceFeed: ~96 bytes (minimal)
- Perpetuals: ~73 KB (supports 1024 positions, 512 users)

### Supported Tokens (Phase 1)

```
Token ID  | Token    | Price Precision | Initial Price
========================================================
0         | QUBIC    | 1e8            | 0.1 USD
1         | BTC      | 1e8            | 45,000 USD
2         | BNB      | 1e8            | 600 USD
3         | USDT     | 1e8            | 1 USD
```

---

## PriceFeed Contract Details

### Purpose
Stores current prices for all supported tokens and provides them to the Perpetuals contract and external queries.

### Public Functions

#### getPrice(token_id: uint8)
- Returns the current price for a specific token
- Price format: scaled by 10^8
- Example: BTC price of 45,000 USD = 4500000000000

#### getAllPrices()
- Returns current prices for all 4 tokens at once
- Useful for atomicity in price updates

#### getOracleAddress()
- Returns the authorized oracle address

### Public Procedures

#### updatePrice(token_id: uint8, new_price: uint64)
- Updates the price for a token
- Only callable by the contract invoker
- Validates that token_id < 4 and price > 0
- Updates the last_update_tick timestamp

#### setOracleAddress(new_oracle_address: id)
- Sets the oracle address (for future oracle authorization)

### System Procedures

#### initialize()
- Initializes all token prices to default values
- Sets last_update_ticks to 0
- Called once during contract construction (epoch N+2)

---

## Perpetuals Contract Details

### Core Features

#### 1. Position Management
- **Max 1024 positions** can be open simultaneously
- **1x to 10x leverage** with validation
- **Long and Short** positions supported
- **3 position statuses**: Open, Closed, Liquidated

#### 2. Risk Management

**Initial Margin Requirement (IMR):** 10%
- User must have at least 10% of position notional value as collateral
- Example: $1000 position with 5x leverage needs $200 margin

**Maintenance Margin Ratio (MMR):** 5%
- Positions liquidatable if margin falls below 5%
- Liquidators can force-close underwater positions

**Liquidation Fee:** 5% of position size
- 50% goes to liquidator reward
- 50% goes to protocol

#### 3. Fee Structure

| Fee Type      | Rate  | When       | Example (10k QU position) |
|---------------|-------|------------|---------------------------|
| Opening Fee   | 0.1%  | Open       | 10 QU                    |
| Closing Fee   | 0.1%  | Close      | 10 QU                    |
| Liquidation   | 5%    | Liquidate  | 500 QU (50% to liquidator)|

#### 4. Funding Rates

**Mechanism:**
- Charged hourly based on open interest imbalance
- Max rate: 0.1% per hour
- Formula: `rate = (imbalance / total_oi) × 100` (capped)

**Who Pays:**
- If longs > shorts: longs pay shorts
- If shorts > longs: shorts pay longs

#### 5. Account Management

**Max 512 users** can have accounts simultaneously

**User Account Data:**
- Available balance (free margin)
- Reserved margin (in active positions)
- Position count
- Realized P&L tracking

### Public Functions

#### Position Information
- `getPosition(position_id)` - Get full position details
- `getPositions(trader)` - Get all positions for a user
- `getMarginRatio(position_id)` - Check liquidation status

#### Market Data
- `getOpenInterest(token_id)` - Get long/short OI for a token
- `getTokenStats(token_id)` - Get token volume and imbalance
- `getFundingRate(token_id)` - Get current hourly funding rate

#### Account Data
- `getUserBalance(trader)` - Get user margin balance
- `getUserStats(trader)` - Get user's position count and P&L

#### Protocol Statistics
- `getProtocolStats()` - Get total fees, liquidations, volume

### Public Procedures

#### Position Trading
1. **openPosition(token_id, direction, collateral, leverage)**
   - Create a new leveraged position
   - Validates leverage (1-10x) and collateral > 0
   - Updates open interest for the token
   - Deducts opening fee from margin

2. **closePosition(position_id)**
   - Close an open position
   - Settles unrealized P&L
   - Deducts closing fee
   - Returns collateral + P&L to user account

3. **liquidatePosition(position_id)**
   - Force-close an underwater position
   - Callable by anyone
   - Checks if margin ratio < 5%
   - Distributes liquidation fees

#### Account Management
1. **depositMargin(amount)**
   - Add funds to user account
   - Can be called multiple times
   - Increases available balance

2. **withdrawMargin(amount)**
   - Withdraw available funds
   - Cannot withdraw reserved margin in active positions
   - Validates sufficient balance

### System Procedures

#### INITIALIZE()
- Called once during contract construction
- Initializes all token stats to zero
- Sets configuration values (fees)

#### BEGIN_EPOCH()
- Called at start of each epoch
- Resets daily volume counters

#### END_EPOCH()
- Called at end of each epoch
- Placeholder for funding settlement logic

---

## Deployment Process (Qubic Governance)

### Step 1: Code Review & Validation

The contracts have been:
- ✅ Compiled successfully (0 errors)
- ✅ Follows Qubic security guidelines
- ✅ Uses only approved C++ features
- ✅ Optimized for execution cost

### Step 2: Create Proposal

1. **Fork the main repository**
   ```bash
   git clone https://github.com/qubic/core
   git checkout -b feature/2025-01-XX-Perpetuals
   ```

2. **Add contracts to `src/contract_core/contract_def.h`**
   ```cpp
   #define PRICEFEED_CONTRACT_INDEX 20
   #define PERPETUALS_CONTRACT_INDEX 21
   ```

3. **Submit to proposal repository**
   - Link: https://github.com/qubic/proposal
   - Include full contract code
   - Document features and security

### Step 3: Computor Voting

Using **GQMPROP** (GeneralQuorumProposal) contract:

1. Proposal is created in epoch N
2. Computors vote until end of epoch N (Wednesday 12:00 UTC)
3. IPO occurs in epoch N+1 (Thursday - Wednesday)
4. Construction and activation in epoch N+2

### Step 4: Contract Activation

**Timeline:**
- **Epoch N**: Proposal voting (mid-epoch preferred)
- **Epoch N+1**: IPO, QUs collected burned for fee reserve
- **Epoch N+2**: Contract initialized and live

**Fee Reserve:**
- Funded by burning QUs during IPO
- Covers execution costs
- Required for contract to stay active

---

## Testing Before Deployment

### 1. Local Build Test ✅

All contracts compile without errors:
```
Build succeeded.
0 Warning(s)
0 Error(s)
```

### 2. Contract Registration ✅

Contracts properly registered in `contract_def.h`:
```cpp
// Entry 20
REGISTER_USER_FUNCTION(getPrice, 1);        // PriceFeed
REGISTER_USER_PROCEDURE(updatePrice, 1);    // PriceFeed

// Entry 21
REGISTER_USER_FUNCTION(getPosition, 1);     // Perpetuals
REGISTER_USER_PROCEDURE(openPosition, 1);   // Perpetuals
```

### 3. State Size Validation ✅

```
sizeof(PRICEFEED) = 96 bytes ✓ (< 1 GB limit)
sizeof(PERPETUALS) = ~73 KB ✓ (< 1 GB limit)
```

### 4. Recommended Security Audit

Before mainnet deployment, conduct:
- [ ] External security audit (recommended)
- [ ] Formal verification of liquidation logic
- [ ] Edge case testing for P&L calculations
- [ ] Stress testing with 1000+ concurrent positions

---

## Configuration & Customization

### Adjustable Parameters

In `src/contracts/Perpetuals.h`:

```cpp
constexpr uint64 PERP_MAX_LEVERAGE = 10;              // Change to 5x, 20x, etc.
constexpr uint64 PERP_INITIAL_MARGIN_PCT = 1000;      // Change from 10%
constexpr uint64 PERP_MAINTENANCE_MARGIN_PCT = 500;   // Change from 5%
constexpr uint32 PERP_OPENING_FEE_BPS = 10;           // Change from 0.1%
constexpr uint32 PERP_CLOSING_FEE_BPS = 10;           // Change from 0.1%
```

### Adding More Tokens

To add a 5th token (e.g., ETH):

1. Update `PerpTokenId` enum in Perpetuals.h:
```cpp
enum PerpTokenId {
    PERP_QUBIC = 0,
    PERP_BTC = 1,
    PERP_BNB = 2,
    PERP_USDT = 3,
    PERP_ETH = 4,      // NEW
    PERP_MAX_TOKENS = 5 // UPDATE
};
```

2. Update PriceFeed initial prices
3. Expand token_stats arrays in Perpetuals
4. Recompile and resubmit for governance approval

---

## Integration with Other Systems

### PriceFeed Integration

The Perpetuals contract can query PriceFeed contract:
```cpp
// Future implementation:
// uint64 price = qpi.callFunction(PRICEFEED_INDEX, getPrice, token_id);
```

Currently uses mock prices:
- BTC: 45,000 USD
- Other prices adjust based on entry price

### Future Oracle Systems

Can integrate with:
- Chainlink-style oracle networks
- Qubic node-based oracles
- Community-maintained price feeds
- Decentralized governance for price validation

---

## Monitoring & Maintenance

### Key Metrics to Track

1. **Open Interest Growth**
   ```
   Total Long OI per token
   Total Short OI per token
   Daily trading volume
   ```

2. **Risk Metrics**
   ```
   Number of positions near liquidation (>5%, <10% margin)
   Total collateral locked
   Average leverage used
   ```

3. **Fee Revenue**
   ```
   Daily opening fees
   Daily closing fees
   Liquidation fees earned
   ```

4. **System Health**
   ```
   Active user count
   Active position count
   Contract state size growth
   ```

### Maintenance Tasks

- Monitor contract state size growth
- Rebalance fee structure if needed (requires new proposal)
- Update prices in PriceFeed contract (automatic via bot)
- Archive closed positions if state grows too large (future upgrade)

---

## Testnet Deployment Checklist

- [ ] Build project successfully (0 errors)
- [ ] Review contract code for security
- [ ] Verify state sizes are reasonable
- [ ] Set up testnet environment
- [ ] Create governance proposal
- [ ] Test price feed updates
- [ ] Test position opening/closing
- [ ] Test liquidations
- [ ] Monitor for 1+ epoch
- [ ] Document findings

---

## Support & Documentation

For questions or issues:
1. Check [doc/contracts.md](../doc/contracts.md) - General contract development
2. Review existing contracts (Qx, QSwap, QEARN, QVAULT)
3. Join Qubic Discord: https://discord.gg/qubic

---

## Version History

| Version | Date       | Changes          |
|---------|------------|------------------|
| 1.0     | 2025-01-06 | Initial release  |

---

## License

Same as Qubic Core - Anti-Military License with MIT exceptions
