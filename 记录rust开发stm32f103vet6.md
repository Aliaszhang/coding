# 1. 初体验
## 1. 网上找了很多资料和文章，最后还是参考了这篇文章

[小哥说的很清楚](https://jonathanklimt.de/electronics/programming/embedded-rust/rust-on-stm32-2/)

## 2. 开始依样画葫芦（环境是Win11）

- 根据步骤安装软件
- 添加代码，然后根据自己的板子，修改点灯程序和配置（重在体验用rust开发嵌入式）


```用的野火的板子，三色灯分别是PB5, PB0, PB1
// src/main.rs

// std and main are not available for bare metal software
#![no_std]
#![no_main]

use cortex_m_rt::entry; // The runtime
use embedded_hal::digital::v2::OutputPin; // the `set_high/low`function
use stm32f1xx_hal::{delay::Delay, pac, prelude::*}; // STM32F1 specific functions
#[allow(unused_imports)]
use panic_halt; // When a panic occurs, stop the microcontroller

// This marks the entrypoint of our application. The cortex_m_rt creates some
// startup code before this, but we don't need to worry about this
#[entry]
fn main() -> ! {
    // Get handles to the hardware objects. These functions can only be called
    // once, so that the borrowchecker can ensure you don't reconfigure
    // something by accident.
    let dp = pac::Peripherals::take().unwrap();
    let cp = cortex_m::Peripherals::take().unwrap();

    // GPIO pins on the STM32F1 must be driven by the APB2 peripheral clock.
    // This must be enabled first. The HAL provides some abstractions for
    // us: First get a handle to the RCC peripheral:
    let mut rcc = dp.RCC.constrain();
    // Now we have access to the RCC's registers. The GPIOC can be enabled in
    // RCC_APB2ENR (Prog. Ref. Manual 8.3.7), therefore we must pass this
    // register to the `split` function.
    let mut gpiob = dp.GPIOB.split(&mut rcc.apb2);
    // This gives us an exclusive handle to the GPIOC peripheral. To get the
    // handle to a single pin, we need to configure the pin first. Pin C13
    // is usually connected to the Bluepills onboard LED.
    let mut led_red = gpiob.pb5.into_push_pull_output(&mut gpiob.crl);
    let mut led_green = gpiob.pb0.into_push_pull_output(&mut gpiob.crl);
    let mut led_blue = gpiob.pb1.into_push_pull_output(&mut gpiob.crl);

    // Now we need a delay object. The delay is of course depending on the clock
    // frequency of the microcontroller, so we need to fix the frequency
    // first. The system frequency is set via the FLASH_ACR register, so we
    // need to get a handle to the FLASH peripheral first:
    let mut flash = dp.FLASH.constrain();
    // Now we can set the controllers frequency to 8 MHz:
    let clocks = rcc.cfgr.sysclk(8.mhz()).freeze(&mut flash.acr);
    // The `clocks` handle ensures that the clocks are now configured and gives
    // the `Delay::new` function access to the configured frequency. With
    // this information it can later calculate how many cycles it has to
    // wait. The function also consumes the System Timer peripheral, so that no
    // other function can access it. Otherwise the timer could be reset during a
    // delay.
    let mut delay = Delay::new(cp.SYST, clocks);

    // Now, enjoy the lightshow!
    loop {
        led_red.set_high().ok();
        led_green.set_high().ok();
        led_blue.set_low().ok();
        delay.delay_ms(1_000_u16);
        led_red.set_low().ok();
        led_green.set_high().ok();
        led_blue.set_high().ok();
        delay.delay_ms(1_000_u16);
        led_red.set_high().ok();
        led_green.set_low().ok();
        led_blue.set_high().ok();
        delay.delay_ms(1_000_u16);
    }
}
```

```memory.x
/* memory.x - Linker script for the STM32F103VET6 */

MEMORY
{
  /* Flash memory begins at 0x80000000 and has a size of 512kB*/
  FLASH : ORIGIN = 0x08000000, LENGTH = 512K
  /* RAM begins at 0x20000000 and has a size of 64kB*/
  RAM : ORIGIN = 0x20000000, LENGTH = 64K
}
```

```Cargo.toml
[package]
name = "rusty-blink"
version = "0.1.0"
edition = "2021"

# See more keys and their definitions at https://doc.rust-lang.org/cargo/reference/manifest.html

[profile.release]
opt-level = 3   # turn on maximum optimizations. We only have 64kB
lto = true      # Link-time-optimizations for further size reduction

[dependencies]
cortex-m = "^0.6.3"      # Access to the generic ARM peripherals
cortex-m-rt = "^0.6.12"  # Startup code for the ARM Core
embedded-hal = "^0.2.4"  # Access to generic embedded functions (`set_high`)
panic-halt = "^0.2.0"    # Panic handler

# Access to the stm32f103 HAL.
[dependencies.stm32f1xx-hal]
# 野火指南者F103 contains a 512kB flash variant which is called "high density"
features = ["stm32f103", "rt", "high"]
version = "^0.6.1"
```
## 3. 编译
```
$ cargo build --release
   Compiling rusty-blink v0.1.0 (D:\rust_workspace\rusty-blink)
    Finished release [optimized] target(s) in 0.48s
warning: the following packages contain code that will be rejected by a future version of Rust: stm32f1xx-hal v0.6.1
note: to see what the problems were, use the option `--future-incompat-report`, or run `cargo report future-incompatibilities --id 2`
```

## 4. 烧录

- cargo flash 报错不能直接用，要给J-Link安装Zadig，但是会替换当前的 SEGGER J-Link driver。这样可能会影响我开发C，所以不安装
  
```
$ cargo flash --chip stm32f103C8 --release
    Finished release [optimized] target(s) in 0.10s
warning: the following packages contain code that will be rejected by a future version of Rust: stm32f1xx-hal v0.6.1
note: to see what the problems were, use the option `--future-incompat-report`, or run `cargo report future-incompatibilities --id 2`
    Flashing D:\rust_workspace\rusty-blink\target\thumbv7m-none-eabi\release\rusty-blink
       Error Failed to open the debug probe.

  Caused by:
          0: USB Communication Error
          1: error while taking control over USB device: interface not found or not compatible(this error may be caused by not having the WinUSB driver installed;
        use Zadig (https://zadig.akeo.ie/) to install it for the J-Link device; this will replace the SEGGER J-Link driver)

        Hint This could be a permission issue. Check our guide on how to make all probes work properly on your system: https://probe.rs/docs/getting-started/probe-setup

```

- 考虑生成bin或者hex，用J-Flash烧录
- 给win11安装`Arm GNU Toolchain + Cygwin + Make`
[Arm GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)
[Cygwin](https://cygwin.com/install.html)

- 进入生成的文件目录`.../rusty-blink/target/thumbv7m-none-eabi/release`
- 生成hex：`arm-none-eabi-objcopy -O ihex rusty-blink blink-rust.hex` 或者bin:` arm-none-eabi-objcopy -O binary rusty-blink blink-rust.bin`
- J-Flash 创建工程并打开烧录即可

## 5. 缺点：不能在线调试。

# 2. 修改使用最新的crate

## 1. 修改`Cargo.toml`

```
[package]
name = "rusty-blink"
version = "0.1.0"
edition = "2021"

# See more keys and their definitions at https://doc.rust-lang.org/cargo/reference/manifest.html

[profile.release]
opt-level = 3   # turn on maximum optimizations. We only have 64kB
lto = true      # Link-time-optimizations for further size reduction

[dependencies]
cortex-m = "0.7.7"      # Access to the generic ARM peripherals
cortex-m-rt = "0.7.4"  # Startup code for the ARM Core
embedded-hal = "1.0.0"  # Access to generic embedded functions (`set_high`)
panic-halt = "^0.2.0"    # Panic handler

# Access to the stm32f103 HAL.
[dependencies.stm32f1xx-hal]
# 野火指南者F103 contains a 512kB flash variant which is called "high density"
features = ["stm32f103", "rt", "high"]
version = "0.10.0"
```

## 2. 修改代码`main.rs`

```

// std and main are not available for bare metal software
#![no_std]
#![no_main]

use cortex_m_rt::entry; // The runtime
use stm32f1xx_hal::{pac, prelude::*}; // STM32F1 specific functions
#[allow(unused_imports)]
use panic_halt; // When a panic occurs, stop the microcontroller

// This marks the entrypoint of our application. The cortex_m_rt creates some
// startup code before this, but we don't need to worry about this
#[entry]
fn main() -> ! {
    let dp = pac::Peripherals::take().unwrap();
    let cp = cortex_m::Peripherals::take().unwrap();

    let rcc = dp.RCC.constrain();
    let mut gpiob = dp.GPIOB.split();
    let mut led_red = gpiob.pb5.into_push_pull_output(&mut gpiob.crl);
    let mut led_green = gpiob.pb0.into_push_pull_output(&mut gpiob.crl);
    let mut led_blue = gpiob.pb1.into_push_pull_output(&mut gpiob.crl);
    let mut flash = dp.FLASH.constrain();
    let clocks = rcc.cfgr.freeze(&mut flash.acr);
    let mut delay = cp.SYST.delay(&clocks);
    
    // Now, enjoy the lightshow!
    loop {
        led_red.set_high();
        led_green.set_high();
        led_blue.set_low();
        delay.delay_ms(500_u16);
        led_red.set_low();
        led_green.set_high();
        led_blue.set_high();
        delay.delay(1.secs());
        led_red.set_high();
        led_green.set_low();
        led_blue.set_high();
        delay.delay_ms(500_u16);
    }
}
```

## 3. 如何查找需要使用的接口

- 打开 [doc.rs](https://docs.rs/)
- 搜索关键字`stm32f1xx-hal`
- 打开`Crate stm32f1xx_hal`
- 找到`Usage examples`,点击`See the examples folder.`转到GitHub上。
- 选择对应的库版本即可查看很多参考示例




