import asyncio
from bleak import BleakScanner
import time
from datetime import datetime

def initalize_file(log_file):
    with open(log_file, 'w') as f:
        f.write("mac_address,RSSI,epoch_time,iso_time\n")

async def scan(mac_to_listen_for = '54:32:04:33:0d:4a', log_file = f"{time.time()}_mac_date_log.csv", timeout = 300):
    initalize_file(log_file)
    print(f"Listening for MAC: {mac_to_listen_for} every {timeout} seconds. Logging to {log_file}")

    while True:
        btle_device = await BleakScanner.find_device_by_address(mac_to_listen_for, timeout)

        if btle_device is not None:
            # timestamps
            current_time = time.time()
            current_iso = datetime.fromtimestamp(current_time).isoformat()

            # extract RSSI and Address
            RSSI = btle_device.details['props']['RSSI']
            address = btle_device.details['props']['Address']

            # append to file
            with open(log_file, 'a') as f:
                print(f"{address},{RSSI},{current_time},{current_iso}")
                f.write(f"{address},{RSSI},{current_time},{current_iso}\n")
        # sleep for the specified timeout duration
        await asyncio.sleep(timeout)

asyncio.run(scan())