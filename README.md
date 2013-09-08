gong-srv
========

Building Images
---------------

This more or less follows the procedure outlined by the OpenWRT [docs](http://wiki.openwrt.org/doc/howto/build).  Note that we insert some patches to customize the default configuration and download a very specific revision.  These instructions were written with a 64-bit Ubuntu 12.04 Server VM for the build.  

1. Checkout OpenWRT revision 29592.

        svn co svn://svn.openwrt.org/openwrt/trunk@29592

1. Install build dependencies

        sudo apt-get install build-essential subversion libncurses5-dev zlib1g-dev gawk gcc-multilib flex git-core gettext

1. Place the files in patches/ into the `target/linux/generic/patches-2.6.37/` directory.

1. Customize and build.  In menuconfig you need to set Kernel modules/Other modules/kmod-pwm-gpio-custom as a built-in package.

        ./scripts/feeds update -a
        make menuconfig
        make tools/quilt/install
        make kernel_menuconfig 
        make defconfig
        make -j 3 # Set to <your number of CPU cores + 1>

1. Flash the images from the /bin directory.


Controlling a servo
-------------------

        # Set GPIO numbers
        export G_PWR=3 # Servo power
        export G_SW=7 # Trigger switch (unused in this case)
        export G_CTL=1 # PWM control line
        
        # Export GPIOs and set direction
        echo $G_PWR > /sys/class/gpio/export
        echo $G_SW > /sys/class/gpio/export
        echo out > /sys/class/gpio/gpio$G_PWR/direction
        echo in > /sys/class/gpio/gpio$G_SW/direction
        
        # Create the PWM device and configure parameters
        insmod pwm-gpio-custom bus0=0,$G_CTL 
        cat /sys/class/pwm/gpio_pwm.0\:0/request # Start ticking
        echo 20000000 > /sys/class/pwm/gpio_pwm.0\:0/period_ns # 20ms period (50hz)
        echo 1200000 > /sys/class/pwm/gpio_pwm.0\:0/duty_ns # 1.2ms ON period
        echo 1 > /sys/class/pwm/gpio_pwm.0\:0/polarity # Make duty cycles more consistent with servo docs
        
        # Start PWM and configure parameters
        echo 1 > /sys/class/pwm/gpio_pwm.0\:0/run
        echo 1 > /sys/class/gpio/gpio$G_PWR/value
        
        # Move across the servo range
        echo 700000 > /sys/class/pwm/gpio_pwm.0\:0/duty_ns 
        sleep 2
        echo 1200000 > /sys/class/pwm/gpio_pwm.0\:0/duty_ns 
        sleep 2
        echo 2100000 > /sys/class/pwm/gpio_pwm.0\:0/duty_ns 
        sleep 2
        echo 1200000 > /sys/class/pwm/gpio_pwm.0\:0/duty_ns 
        sleep 1
        
        # Turn the power off
        echo 0 > /sys/class/gpio/gpio$G_PWR/value
        echo 0 > /sys/class/pwm/gpio_pwm.0\:0/run

