WeatherLog
==========

WeatherLog is designed to make recording and analysing your own weather data a quick and easy experience. Load the Arduino firmware and start recording data to a PC (running just-about any OS you like), then use the Mac app to analyse the recorded data and produce easy-to-understand tables and graphs.


##Features
WeatherLog is in its infancy, so features are still limited. See the roadmap, below for a taste of what's to come. Currently, WeatherLog will:

* Record temperature and relative humidity (RH) every five seconds
* Transmit data with a GPS-based timestamp over serial link
* Read a recorded data file and produce:
	- A chart of temperature and RH over time
	- The maximum and minimum temperature and RH, and the time of each

### Roadmap
Planned features include:

* Real-time data support: see the current weather on your Mac
* A server app to receive data from the Arduino, log to disk, and supply the Mac app
* Bluetooth Smart support: log data via Bluetooth; see weather data on your iPhone via Bluetooth.
* Multiple device support: log data from more than one location
* Improved sensor support: log wind speed and direction, rainfall, pressure, etc.
* An Arduino shield (open source hardware, of course).

Do note that this is a spare-time project and the above list is not set in stone. Some—or all—of it may not ever happen.


## Requirements
The WeatherLog Arduino firmware has been developed for the [Arduino Uno][], but may work on other Arduino boards; currently, the only supported sensors are the Analog Devices TMP36 analog temperature sensor, and the Honeywell HIH-4030 analog relative humidity sensor.

WeatherLog for Mac requires OS X 10.8 or later; any Mac running Mountain Lion or Mavericks should work just fine.

You'll also need a way to get the data from the Arduino, which outputs CSV data over a serial connection, to the Mac app, which reads a CSV file from disk. You can use commonly-available UNIX utilities like `stty`, `cat`, and (optionally) `grep` to accomplish this.

[Arduino Uno]: http://arduino.cc/en/Main/ArduinoBoardUno


## License
WeatherLog is open source; see the [LICENSE.md file](LICENSE.md) for full details (it's the Apache license). The WeatherLog name and logo are trademarks of Greg Waterhouse and may not be used without permission.

Portions of this software may belong to third parties and used under license. See the [NOTICE.md file](NOTICE.md) for details.
