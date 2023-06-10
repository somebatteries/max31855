/*----------------------------------------------------------
spi data format:
D[31:18] 14-Bit Thermocouple Temperature Data These bits contain the signed 14-bit thermocouple temperature value. See Table 4.
D17      Reserved This bit always reads 0.
D16      Fault    This bit reads at 1 when any of the SCV, SCG, or OC faults are active. Default value is 0.
D[15:4]  12-Bit Internal Temperature Data These bits contain the signed 12-bit value of the reference junction temperature. See
D3       Reserved This bit always reads 0.
D2       SCV Fault This bit is a 1 when the thermocouple is short-circuited to VCC. Default value is 0.
D1       SCG Fault This bit is a 1 when the thermocouple is short-circuited to GND. Default value is 0.
D0       OC Fault This bit is a 1 when the thermocouple is open (no connections). Default value is 0
----------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#define SPI_DEVICE "/dev/spidev0.0"

struct max31855_result {
    int16_t thermocouple_temp;
    int16_t internal_temp;
    uint8_t fault;
    uint8_t scv_fault;
    uint8_t scg_fault;
    uint8_t oc_fault;
};

int main(int argc, char** argv) {
    int spi_fd;
    uint8_t spi_data[4];
    struct spi_ioc_transfer spi_transfer = {
        .tx_buf = 0,
        .rx_buf = (unsigned long)spi_data,
        .len = sizeof(spi_data),
        .speed_hz = 500000,
        .bits_per_word = 8,
    };

    const char * dev = argc > 1 ? argv[1] : SPI_DEVICE;

    spi_fd = open(dev, O_RDWR);
    if (spi_fd < 0) {
        perror("Error opening SPI device");
        return -1;
    }

    if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &spi_transfer) < 0) {
        perror("Error during SPI transfer");
        close(spi_fd);
        return -1;
    }

    close(spi_fd);

    struct max31855_result result;
    result.thermocouple_temp = (int16_t)((spi_data[0] << 8) | spi_data[1]) >> 2;
    result.internal_temp = (int16_t)((spi_data[2] << 8) | spi_data[3]) >> 4;
    result.fault = (spi_data[1] & 0x01) ? 1 : 0;
    result.scv_fault = (spi_data[3] & 0x04) ? 1 : 0;
    result.scg_fault = (spi_data[3] & 0x02) ? 1 : 0;
    result.oc_fault = (spi_data[3] & 0x01) ? 1 : 0;

    if (result.fault || result.scv_fault || result.scg_fault || result.oc_fault) {
        perror("Fault");
        return -1;
    }

    float thermocouple_temp_float = (float)result.thermocouple_temp * 0.25;

    printf("%.2f\n", thermocouple_temp_float);

    return 0;
}

