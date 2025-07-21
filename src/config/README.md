# Configuration repository for vSID in LFXX airspace

## Current status
[![JSON Validation](https://github.com/vaccfr/vsid-settings/actions/workflows/valid_json.yml/badge.svg)](https://github.com/vaccfr/vsid-settings/actions/workflows/valid_json.yml)

## vSID Plugin

- [vSID GitHub Wiki](https://github.com/Gameagle/vSID/wiki)
- [vSID on Vatsim Germany KB](https://knowledgebase.vatsim-germany.org/books/vsid-plugin/page/vsid-controller-manual)

> [!CAUTION]
> The configuration files are now only compatible with vSID v0.12.0 and above

## Supported platforms

| FIR | Platforms |
| --- | --- |
| LFBB | LFBA LFBD LFBE LFBF LFBG LFBH LFBI LFBL LFBO LFBP LFBT LFBU LFBZ LFCK LFCR LFDN |
| LFEE | LFJL LFSB LFSL LFST |
| LFFF | LFOA LFOB LFOJ LFOK LFOP LFOT LFAQ LFAT LFPB LFPG LFPM LFPN LFPO LFPT LFPV LFQB LFQQ LFQT |
| LFMM | LFKB LFKC LFKF LFKJ LFLB LFLC LFLL LFLN LFLP LFLS LFLX LFMD LFMH LFMI LFMK LFML LFMN LFMP LFMQ LFMT LFMU LFMV LFTH LFTW LFTZ |
| LFRR | LFRB LFRD LFRG LFRH LFRK LFRN LFRQ LFRZ | 

## Custom rules

Customs rules are used in Paris airports to specify the configuration linked or inverted.

For LFPG, LFPB and LFPO, the rule `opposing` sets the configuration:
- `opposing=OFF`: Linked configuration (default)
- `opposing=ON`: Opposing configuration

For LFOB, the rule `pgeast` sets the configuration:
- `pgeast=OFF`: LFPG in West configuration (default)
- `pgeast=ON`: LFPG in East configuration

For LFPN, LFPT and LFPV, one of the 4 rules MUST manually be enabled. Not enabling any rule will result in no SID automatic assignment.
- `wlpg=ON`: LFPG in West linked configuration
- `elpg=ON`: LFPG in East linked configuration
- `wipg=ON`: LFPG in West opposing configuration
- `eipg=ON`: LFPG in East opposing configuration

To toggle a rule: `.vsid rule <airport icao> <rule name>`

## Areas

 The 2 areas `north` and `south` at LFPG are used to manage the taxi configuration:
- `ON`: Roulage mini (default)
- `OFF`: Croisement au sol

The 2 areas **MUST** be both ON or OFF at the same time. To toggle an area:
- `.vsid area LFPG north`
- `.vsid area LFPG south`
