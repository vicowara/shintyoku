#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h> // alloc_chrdev_region
#include <linux/cdev.h> // cdev_init
#include <linux/uaccess.h> // copy_from_user, copy_to_user

#define MODNAME "shintyoku"

#define MINOR_COUNT 1 // 接続するマイナー番号数

static dev_t dev_id;  // デバイス番号
static struct cdev c_dev; // キャラクタデバイス構造体

// 内部バッファ
#define MAX_BUFLEN 64
static unsigned char cdev_buf[MAX_BUFLEN];
static int cdev_buflen;

// スペシャルファイルをオープンした
static int cdev_open(struct inode *inode, struct file *filp)
{
	pr_info("cdev_open\n");
	return 0;
}


// スペシャルファイルをクローズした
static int cdev_release(struct inode *inode, struct file *filp)
{
	pr_info("cdev_release\n");
	return 0;
}


// スペシャルファイルからの読み込み
static ssize_t cdev_read(struct file *filp, char *buf, size_t count, loff_t *offset)
{
	int len;
	char text[MAX_BUFLEN];

	if (cdev_buflen == 0) {
		len = sprintf(text, "shintyoku\n");
	} else {
		len = sprintf(text, "damedesu\n");
	}
	pr_info("cdev_read count = %lu\n", count);

	if (copy_to_user(buf, text, len)) {
		pr_warn("copy_to_user failed\n");
		return -EFAULT;
	}

	*offset += len;
	cdev_buflen = 0;

	return len;
}


// スペシャルファイルへの書き込み
static ssize_t cdev_write(struct file *filp, const char *buf, size_t count, loff_t *offset)
{
	if (count >= MAX_BUFLEN) {
		return -EFAULT;
	}

	pr_info("cdev_write count = %lu\n", count);

	if (copy_from_user(cdev_buf, buf, count)) {
		pr_warn("copy_from_user failed\n");
		return -EFAULT;
	}

	cdev_buf[count] = '\0';
	pr_info("cdev_buf = %s\n", cdev_buf);

	*offset += count;
	cdev_buflen = count;

	return count;
}


// ファイルオペレーション構造体
// スペシャルファイルに対して読み書きなどを行ったときに呼び出す関数を登録する
static const struct file_operations modtest_fops = {
	.owner   = THIS_MODULE,
	.open    = cdev_open,
	.release = cdev_release,
	.read    = cdev_read,
	.write   = cdev_write,
};


// モジュール初期化
static int __init modtest_module_init(void)
{
	int ret;

	// キャラクタデバイス番号の動的取得
	ret = alloc_chrdev_region(
			&dev_id, // 最初のデバイス番号が入る
			0,  // マイナー番号の開始番号
			MINOR_COUNT, // 取得するマイナー番号数
			MODNAME // モジュール名
			);
	if (ret < 0) {
		pr_warn("alloc_chrdev_region failed\n");
		return ret;
	}

	// キャラクタデバイス初期化
	// ファイルオペレーション構造体の指定もする
	cdev_init(&c_dev, &modtest_fops);
	c_dev.owner = THIS_MODULE;

	// キャラクタデバイスの登録
	// MINOR_COUNT が 1 でマイナー番号の開始番号が 0 なので /dev/modtest0 が
	// 対応するスペシャルファイルになる
	ret = cdev_add(&c_dev, dev_id, MINOR_COUNT);
	if (ret < 0) {
		pr_warn("cdev_add failed\n");
		return ret;
	}

	pr_info("modtest is loaded\n");
	pr_info("major = %d\n", MAJOR(dev_id));
	pr_info("minor = %d\n", MINOR(dev_id));

	return 0;
}


//  モジュール解放
static void __exit modtest_module_exit(void)
{
	// キャラクタデバイス削除
	cdev_del(&c_dev);

	// デバイス番号の返却
	unregister_chrdev_region(dev_id, MINOR_COUNT);

	pr_info("modtest is removed\n");
}


module_init(modtest_module_init);
module_exit(modtest_module_exit);

MODULE_DESCRIPTION(MODNAME);
MODULE_LICENSE("GPL v2");
