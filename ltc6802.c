#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spi/spi.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/device.h>

#include <linux/iio/iio.h>
#include <linux/iio/buffer.h>
#include <linux/iio/trigger.h>
#include <linux/iio/trigger_consumer.h>
#include <linux/iio/triggered_buffer.h>

/* LTC6802 Commands */
LTC6802_CMD_WRCFG		0x01
LTC6802_CMD_RDCFG		0x02
LTC6802_CMD_RDCV		0x04
LTC6802_CMD_RDFLG		0x06
LTC6802_CMD_RDTMP		0x08
LTC6802_CMD_STCVAD		0x10
  LTC6802_CMD_STCVAD_CELL1	0x11
  LTC6802_CMD_STCVAD_CELL2	0x12
  LTC6802_CMD_STCVAD_CELL3	0x13
  LTC6802_CMD_STCVAD_CELL4	0x14
  LTC6802_CMD_STCVAD_CELL5	0x15
  LTC6802_CMD_STCVAD_CELL6	0x16
  LTC6802_CMD_STCVAD_CELL7	0x17
  LTC6802_CMD_STCVAD_CELL8	0x18
  LTC6802_CMD_STCVAD_CELL9	0x19
  LTC6802_CMD_STCVAD_CELL10	0x1A
  LTC6802_CMD_STCVAD_CELL11	0x1B /* if CELL10 bit = 0 */
  LTC6802_CMD_STCVAD_CELL12	0x1C /* if CELL10 bit = 0 */	
  LTC6802_CMD_STCVAD_TEST1	0x1E
  LTC6802_CMD_STCVAD_TEST2	0x1F
LTC6802_CMD_STOWAD		0x10
  LTC6802_CMD_STCVAD_CELL1	0x21
  LTC6802_CMD_STCVAD_CELL2	0x22
  LTC6802_CMD_STCVAD_CELL3	0x23
  LTC6802_CMD_STCVAD_CELL4	0x24
  LTC6802_CMD_STCVAD_CELL5	0x25
  LTC6802_CMD_STCVAD_CELL6	0x26
  LTC6802_CMD_STCVAD_CELL7	0x27
  LTC6802_CMD_STCVAD_CELL8	0x28
  LTC6802_CMD_STCVAD_CELL9	0x29
  LTC6802_CMD_STCVAD_CELL10	0x2A
  LTC6802_CMD_STCVAD_CELL11	0x2B /* if CELL10 bit = 0 */
  LTC6802_CMD_STCVAD_CELL12	0x2C /* if CELL10 bit = 0 */
  LTC6802_CMD_STCVAD_TEST1	0x2E
  LTC6802_CMD_STCVAD_TEST2	0x2F
LTC6802_CMD_STTMPAD		0x30
  LTC6802_CMD_STTMPAD_ETEMP1	0x31
  LTC6802_CMD_STTMPAD_ETEMP2	0x32
  LTC6802_CMD_STTMPAD_ITEMP	0x33
  LTC6802_CMD_STTMPAD_TEST1	0x3E
  LTC6802_CMD_STTMPAD_TEST2	0x3F
LTC6802_CMD_PLADC		0x40 
LTC6802_CMD_PLINT		0x50
LTC6802_CMD_STCVDC		0x60
  LTC6802_CMD_STCVDC_CELL1	0x61
  LTC6802_CMD_STCVDC_CELL2	0x62
  LTC6802_CMD_STCVDC_CELL3	0x63
  LTC6802_CMD_STCVDC_CELL4	0x64
  LTC6802_CMD_STCVDC_CELL5	0x65
  LTC6802_CMD_STCVDC_CELL6	0x66
  LTC6802_CMD_STCVDC_CELL7	0x67
  LTC6802_CMD_STCVDC_CELL8	0x68
  LTC6802_CMD_STCVDC_CELL9	0x69
  LTC6802_CMD_STCVDC_CELL10	0x6A
  LTC6802_CMD_STCVDC_CELL11	0x6B /* if CELL10 bit = 0 */
  LTC6802_CMD_STCVDC_CELL12	0x6C /* if CELL10 bit = 0 */	
  LTC6802_CMD_STCVDC_TEST1	0x6E
  LTC6802_CMD_STCVDC_TEST2	0x6F
LTC6802_CMD_STOWDC		0x70
  LTC6802_CMD_STOWDC_CELL1	0x71
  LTC6802_CMD_STOWDC_CELL2	0x72
  LTC6802_CMD_STOWDC_CELL3	0x73
  LTC6802_CMD_STOWDC_CELL4	0x74
  LTC6802_CMD_STOWDC_CELL5	0x75
  LTC6802_CMD_STOWDC_CELL6	0x76
  LTC6802_CMD_STOWDC_CELL7	0x77
  LTC6802_CMD_STOWDC_CELL8	0x78
  LTC6802_CMD_STOWDC_CELL9	0x79
  LTC6802_CMD_STOWDC_CELL10	0x7A
  LTC6802_CMD_STOWDC_CELL11	0x7B /* if CELL10 bit = 0 */
  LTC6802_CMD_STOWDC_CELL12	0x7C /* if CELL10 bit = 0 */	
  LTC6802_CMD_STOWDC_TEST1	0x7E
  LTC6802_CMD_STOWDC_TEST2	0x7F

static ssize_t test = 0;

enum ltc6802_id {
	ltc6802
};

static const struct spi_device_id ltc6802_id[] = {
	{"ltc6802", ltc6802},
	{}
};
MODULE_DEVICE_TABLE(spi, ltc6802_id);

#ifdef CONFIG_OF
static const struct of_device_id ltc6802_adc_dt_ids[] = {
	{ .compatible = "linear,ltc6802" },
	{},
};
MODULE_DEVICE_TABLE(of, ltc6802_adc_dt_ids);
#endif

#define LTC6802_V_CHAN(index)						\
	{								\
		.type = IIO_VOLTAGE,					\
		.indexed = 1,						\
		.channel = index,					\
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),		\
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_SCALE),	\
		.scan_index = index + 1,				\
		.scan_type = {						\
			.sign = 'u',					\
			.realbits = 12,					\
			.storagebits = 16,				\
			.endianness = IIO_BE,				\
		},							\
	}

#define LTC6802_T_CHAN(index)						\
	{								\
		.type = IIO_TEMP,					\
		.indexed = 1,						\
		.channel = index,					\
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),		\
		.info_mask_shared_by_all = BIT(IIO_CHAN_INFO_SCALE),	\
		.scan_index = index + 1,				\
		.scan_type = {						\
			.sign = 'u',					\
			.realbits = 12,					\
			.storagebits = 16,				\
			.endianness = IIO_BE,				\
		},							\
	}

static const struct iio_chan_spec ltc6802_channels[] = {
	LTC6802_T_CHAN(0), /* Internal temperature */
	LTC6802_T_CHAN(1),
	LTC6802_T_CHAN(2),
	LTC6802_V_CHAN(1), /* Start at 1 to match cells numbering */
	LTC6802_V_CHAN(2),
	LTC6802_V_CHAN(3),
	LTC6802_V_CHAN(4),
	LTC6802_V_CHAN(5),
	LTC6802_V_CHAN(6),
	LTC6802_V_CHAN(7),
	LTC6802_V_CHAN(8),
	LTC6802_V_CHAN(9),
	LTC6802_V_CHAN(10),
	LTC6802_V_CHAN(11),
	LTC6802_V_CHAN(12),
};

struct ltc6802_chip_info {
	const struct iio_chan_spec *channels;
	unsigned int num_channels;
};

static const struct ltc6802_chip_info ltc6802_chip_info_tbl[] = {
	[ltc6802] = {
		.channels = ltc6802_channels,
		.num_channels = ARRAY_SIZE(ltc6802_channels),
	},
};

struct ltc6802_state {
	const struct ltc6802_chip_info	*info;
	struct spi_device		*spi;
	__be16				*buffer;
	struct mutex			lock;
	u8				reg ____cacheline_aligned;
};

static int ltc6802_read_raw(struct iio_dev *indio_dev,
			    struct iio_chan_spec const *chan,
			    int *val, int *val2, long mask)
{
	int ret = 0;
	struct ltc6802_state *st = iio_priv(indio_dev);
	u8 cfg[7] = {0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00};
	u8 buf_tx[2] = {0x80, 0x02};
	u8 buf_rx[7];
	struct spi_transfer t[] = {
		{
			.tx_buf = cfg,
			.len = 7,
			.cs_change = 1,
		}, {
			.tx_buf = buf_tx,
			.len = 2,
		}, {
			.rx_buf = buf_rx,
			.len = 7,
		},
	};

	mutex_lock(&st->lock);

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		//ret = ltc6802_read_single_value(indio_dev, chan, val);
		//ret = spi_write(st->spi, buf_tx, sizeof(buf_tx));
		//ret = spi_read(st->spi, buf_rx, sizeof(buf_rx));
		ret = spi_sync_transfer(st->spi, t, ARRAY_SIZE(t));
		ret = IIO_VAL_INT;
		*val = 33000;
		break;
	case IIO_CHAN_INFO_SCALE:
		switch (chan->type) {
		case IIO_TEMP:
		case IIO_VOLTAGE:
			*val = 3065 * 2;
			*val2 = 12;
			ret = IIO_VAL_FRACTIONAL_LOG2;
			break;
		default:
			ret = -EINVAL;
			break;
		}
		break;
	default:
		ret = -EINVAL;
		break;
	}

	mutex_unlock(&st->lock);

	return ret;
}

static const struct iio_info ltc6802_info = {
	.driver_module = THIS_MODULE,
	.read_raw = &ltc6802_read_raw,
};

static ssize_t digital_io_show(struct device *dev,
                               struct device_attribute *attr, char *buf)
{
	pr_info("%s\n", attr->attr.name);
	return sprintf(buf, "%d\n", test);
}
static ssize_t digital_io_store(struct device *dev,
                                struct device_attribute *attr, const char *buf,
                                size_t count)
{
	pr_info("%s\n", attr->attr.name);
	sscanf(buf, "%d\n", &test);
	return count;
}

static DEVICE_ATTR(cell1_bypass,  S_IWUSR | S_IRUGO, digital_io_show, digital_io_store);
static DEVICE_ATTR(cell2_bypass,  S_IWUSR | S_IRUGO, digital_io_show, digital_io_store);
static DEVICE_ATTR(cell3_bypass,  S_IWUSR | S_IRUGO, digital_io_show, digital_io_store);
static DEVICE_ATTR(cell4_bypass,  S_IWUSR | S_IRUGO, digital_io_show, digital_io_store);
static DEVICE_ATTR(cell5_bypass,  S_IWUSR | S_IRUGO, digital_io_show, digital_io_store);
static DEVICE_ATTR(cell6_bypass,  S_IWUSR | S_IRUGO, digital_io_show, digital_io_store);
static DEVICE_ATTR(cell7_bypass,  S_IWUSR | S_IRUGO, digital_io_show, digital_io_store);
static DEVICE_ATTR(cell8_bypass,  S_IWUSR | S_IRUGO, digital_io_show, digital_io_store);
static DEVICE_ATTR(cell9_bypass,  S_IWUSR | S_IRUGO, digital_io_show, digital_io_store);
static DEVICE_ATTR(cell10_bypass, S_IWUSR | S_IRUGO, digital_io_show, digital_io_store);
static DEVICE_ATTR(cell11_bypass, S_IWUSR | S_IRUGO, digital_io_show, digital_io_store);
static DEVICE_ATTR(cell12_bypass, S_IWUSR | S_IRUGO, digital_io_show, digital_io_store);

static DEVICE_ATTR(gpio1_value, S_IWUSR | S_IRUGO, digital_io_show, digital_io_store);
static DEVICE_ATTR(gpio2_value, S_IWUSR | S_IRUGO, digital_io_show, digital_io_store);

static DEVICE_ATTR(gpio1_direction, S_IWUSR | S_IRUGO, digital_io_show, digital_io_store);
static DEVICE_ATTR(gpio2_direction, S_IWUSR | S_IRUGO, digital_io_show, digital_io_store);

static struct attribute *dev_attrs[] = {
	&dev_attr_cell1_bypass.attr,
	&dev_attr_cell2_bypass.attr,
	&dev_attr_cell3_bypass.attr,
	&dev_attr_cell4_bypass.attr,
	&dev_attr_cell5_bypass.attr,
	&dev_attr_cell6_bypass.attr,
	&dev_attr_cell7_bypass.attr,
	&dev_attr_cell8_bypass.attr,
	&dev_attr_cell9_bypass.attr,
	&dev_attr_cell10_bypass.attr,
	&dev_attr_cell11_bypass.attr,
	&dev_attr_cell12_bypass.attr,
	&dev_attr_gpio1_value.attr,
	&dev_attr_gpio2_value.attr,
	&dev_attr_gpio1_direction.attr,
	&dev_attr_gpio2_direction.attr,
	NULL,
};
ATTRIBUTE_GROUPS(dev);

static int ltc6802_probe(struct spi_device *spi)
{
	int ret;
	struct iio_dev *indio_dev;
	struct ltc6802_state *st;

	pr_info("%s: probe(spi = 0x%p)\n", __func__, spi);

	indio_dev = devm_iio_device_alloc(&spi->dev, sizeof(*st));
	if (indio_dev == NULL) {
		pr_err("Can't allocate iio device\n");
		return -ENOMEM;
	}

	spi_set_drvdata(spi, indio_dev);

	st = iio_priv(indio_dev);
	st->spi = spi;
	st->info = &ltc6802_chip_info_tbl[spi_get_device_id(spi)->driver_data];

	mutex_init(&st->lock);

	indio_dev->name = spi_get_device_id(spi)->name;
	indio_dev->dev.parent = &spi->dev;
	indio_dev->info = &ltc6802_info;
	indio_dev->modes = INDIO_DIRECT_MODE;
	indio_dev->channels = st->info->channels;
	indio_dev->num_channels = st->info->num_channels;
	/* indio_dev->groups[0] is already used by &indio_dev->chan_attr_group */
	indio_dev->groups[1] = dev_groups[0];

	st->buffer = devm_kmalloc(&indio_dev->dev,
				  indio_dev->num_channels * 2,
				  GFP_KERNEL);
	if (st->buffer == NULL) {
		dev_err(&indio_dev->dev, "Can't allocate buffer\n");
		return -ENOMEM;
	}

	ret = iio_device_register(indio_dev);
	if (ret < 0) {
		dev_err(&indio_dev->dev, "Failed to register iio device\n");
		return ret;
	}

	return ret;
}

static int ltc6802_remove(struct spi_device *spi)
{
	struct iio_dev *indio_dev = spi_get_drvdata(spi);

	pr_info("%s: remove(spi = 0x%p)\n", __func__, spi);

	iio_device_unregister(indio_dev);

	return 0;
}

static struct spi_driver ltc6802_driver = {
	.driver = {
		.name	= "ltc6802",
		.of_match_table = of_match_ptr(ltc6802_adc_dt_ids),
	},
	.probe		= ltc6802_probe,
	.remove		= ltc6802_remove,
	.id_table	= ltc6802_id,
};
module_spi_driver(ltc6802_driver);

MODULE_AUTHOR("Olivier C. Larocque <olivier.c.larocque@gmail.com>");
MODULE_DESCRIPTION("LTC6802 Battery Stack Monitor");
MODULE_LICENSE("GPL v2");
