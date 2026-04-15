# PERP Mainnet Submission Checklist

## Core (qubic/core)
- [ ] PERP contracts compile with 0 errors and 0 warnings
- [ ] Contract verification tool passes for `PriceFeed.h` and `Perpetuals.h`
- [ ] Unit tests for both contracts exist and pass (gtest)
- [ ] Core PR targets `develop` and is merged
- [ ] `contract_def.h` construction epochs are set correctly for the intended go-live epoch (X+2)

## Proposals (qubic/proposals)
- [ ] Proposal markdown merged under `SmartContracts/` (link is permanent)
- [ ] Proposal includes core PR link, commit hashes (optional), and testing/verification evidence

## On-Chain (GQMPROP)
- [ ] Computor operator available to submit proposal in Epoch X (submit mid-epoch or earlier)
- [ ] GQMPROP description includes:
  - [ ] Link to merged proposals repo document
  - [ ] Link to merged core PR
- [ ] Voting period communicated to computors (until end of epoch)

## IPO (Epoch X+1)
- [ ] Community outreach plan for IPO (all 676 shares must sell)
- [ ] Clear statement of shareholder value / dividends / revenue model (if applicable)

