## Rust Setup
Here are the steps I did to setup this project: 
0. Getting Rust Setup
```curl --proto '=https' --tlsv1.2 --fail --show-error --silent https://sh.rustup.rs | sh -s -- -y
source "$HOME/.cargo/env"
rustup toolchain install nightly --component rust-src```

1. Installing LDProxy, ESPFlash
```cargo install espflash ldproxy cargo-generate```

2. Get cargo-generate
```cargo install cargo-generate```

3. Generating the project based off Espressif's
```cargo generate --git https://github.com/esp-rs/esp-idf-template cargo```

* Named project `rustfw`, selected `esp32c3', `false` to advanced configuration

4. Build Cargo
```cargo build```

If you have issues with certification, try these commands
```sudo "/Applications/Python 3.12/Install Certificates.command"
"/Users/abenstirling/Documents/git/opentag/Firmware/rustfw/.embuild/espressif/python_env/idf5.1_py3.12_env/bin/python" -m pip install --upgrade certifi
```
