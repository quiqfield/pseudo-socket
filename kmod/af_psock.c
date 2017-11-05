/*
 * af_psock.c: Pseudo Socket
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/net.h>
#include <net/sock.h>

#include <psock.h>

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt


struct pseudo_sock {
	/* struct sock has to be the first member of pseudo_sock */
	struct sock sk;
    struct socket *sock; /* orig_sock */
};


static inline struct pseudo_sock *pseudo_sk(const struct sock *sk)
{
	return (struct pseudo_sock *)sk;
}


static int psock_release(struct socket *sock)
{
	struct sock *sk = sock->sk;
    struct pseudo_sock *psk;

	if (!sk) {
		pr_debug("%s, NULL sk\n", __func__);
		return 0;
	}
	pr_debug("%s\n", __func__);

    /* Clear orig_sock */
    psk = pseudo_sk(sk);
    if (psk->sock)
        sock_release(psk->sock);

	/* Clear state */
	sock_orphan(sk);
	sk_refcnt_debug_release(sk);
	sock_put(sk);

	sock->sk = NULL;

	return 0;
}

static int psock_bind(struct socket *sock, struct sockaddr *uaddr, int addr_len)
{
	int ret;
	struct pseudo_sock *psk = pseudo_sk(sock->sk);

    if (!uaddr)
        return -EINVAL;

	ret = psk->sock->ops->bind(psk->sock, uaddr, addr_len);
    if (ret) {
        pr_err("%s: psk->sock->ops->bind() failed: %d\n", __func__, ret);
        return ret;
    }

    pr_debug("%s: bind success\n", __func__);

	return 0;
}

static int psock_dgram_connect(struct socket *sock, struct sockaddr *addr,
		int addr_len, int flags)
{
    struct pseudo_sock *psk = pseudo_sk(sock->sk);
    return psk->sock->ops->connect(psk->sock, addr, addr_len, flags);
}

static int psock_getname(struct socket *sock, struct sockaddr *uaddr,
		int *uaddr_len, int peer)
{
    struct pseudo_sock *psk = pseudo_sk(sock->sk);
    return psk->sock->ops->getname(psk->sock, uaddr, uaddr_len, peer);
}

static unsigned int psock_dgram_poll(struct file *file, struct socket *sock,
		poll_table *wait)
{
    struct pseudo_sock *psk = pseudo_sk(sock->sk);
    return psk->sock->ops->poll(file, psk->sock, wait);
}

static int psock_ioctl(struct socket *sock, unsigned int cmd, unsigned long arg)
{
    struct pseudo_sock *psk = pseudo_sk(sock->sk);
    return psk->sock->ops->ioctl(psk->sock, cmd, arg);
}

static int psock_shutdown(struct socket *sock, int mode)
{
    struct pseudo_sock *psk = pseudo_sk(sock->sk);
    return psk->sock->ops->shutdown(psk->sock, mode);
}

static int psock_sendmsg(struct socket *sock, struct msghdr *msg, size_t size)
{
    struct pseudo_sock *psk = pseudo_sk(sock->sk);
    return psk->sock->ops->sendmsg(psk->sock, msg, size);
}

static int psock_recvmsg(struct socket *sock, struct msghdr *msg,
		size_t size, int flags)
{
    struct pseudo_sock *psk = pseudo_sk(sock->sk);
    return psk->sock->ops->recvmsg(psk->sock, msg, size, flags);
}

static ssize_t psock_sendpage(struct socket *sock, struct page *page,
		int offset, size_t size, int flags)
{
    struct pseudo_sock *psk = pseudo_sk(sock->sk);
    return psk->sock->ops->sendpage(psk->sock, page, offset, size, flags);
}



static const struct proto_ops psock_dgram_ops = {
	.family		= PF_PSOCK,
	.owner		= THIS_MODULE,
	.release	= psock_release,
	.bind		= psock_bind,
	.connect	= psock_dgram_connect,
	.socketpair	= sock_no_socketpair,
	.accept		= sock_no_accept,
	.getname	= psock_getname,
	.poll		= psock_dgram_poll,
	.ioctl		= psock_ioctl,
	.listen		= sock_no_listen,
	.shutdown	= psock_shutdown,
	.setsockopt	= sock_common_setsockopt,
	.getsockopt	= sock_common_getsockopt,
	.sendmsg	= psock_sendmsg,
	.recvmsg	= psock_recvmsg,
	.mmap		= sock_no_mmap,
	.sendpage	= psock_sendpage,
	/* .set_peek_off	= sk_set_peek_off, */
};

static struct proto psock_proto = {
	.name	= "psock",
	.owner	= THIS_MODULE,
	.obj_size	= sizeof(struct pseudo_sock),
};


/*
 *  Create a pseudo socket.
 */

static int psock_create(struct net *net, struct socket *sock,
		int protocol, int kern)
{
	int ret;
	struct sock *sk;
	struct pseudo_sock *psk;

	pr_debug("%s\n", __func__);

	switch (sock->type) {
	/* Only SOCK_DGRAM is supported */
	case SOCK_DGRAM:
		sock->ops = &psock_dgram_ops;
		break;
	case SOCK_STREAM:
	default:
		return -ESOCKTNOSUPPORT;
	}

	sk = sk_alloc(net, PF_PSOCK, GFP_KERNEL, &psock_proto, kern);
	if (!sk)
		return -ENOMEM;

	sock_init_data(sock, sk);
    sk->sk_protocol = protocol;

	psk = pseudo_sk(sk);
	psk->sock = NULL;

	/* Create a socket with original family (AF_INET) */
	ret = __sock_create(get_net(&init_net), AF_INET,
			sk->sk_type, sk->sk_protocol, &psk->sock, kern);
	if (ret < 0) {
        pr_err("%s: failed to create a socket with original family\n",
                __func__);
		sk_free(sk);
        return ret;
	}

	return 0;
}

static const struct net_proto_family psock_family_ops = {
	.family	= PF_PSOCK,
	.create	= psock_create,
	.owner	= THIS_MODULE,
};



static int __init af_psock_init(void)
{
	int ret;

	ret = proto_register(&psock_proto, 1);
	if (ret != 0) {
		pr_err("%s: proto_register failed '%d'\n", __func__, ret);
		goto proto_register_failed;
	}

	ret = sock_register(&psock_family_ops);
	if (ret != 0) {
		pr_err("%s: sock_register failed '%d'\n", __func__, ret);
		goto sock_register_failed;
	}

	pr_info("Pseudo Socket (Ver. %s) is loaded\n", PSOCK_VERSION);

	return ret;

sock_register_failed:
	proto_unregister(&psock_proto);
proto_register_failed:
	return ret;
}

static void __exit af_psock_exit(void)
{
	sock_unregister(PF_PSOCK);
	proto_unregister(&psock_proto);

	pr_info("Pseudo Socket (Ver. %s) is unloaded\n", PSOCK_VERSION);
}


module_init(af_psock_init);
module_exit(af_psock_exit);

MODULE_VERSION(PSOCK_VERSION);
MODULE_AUTHOR("Hiroki Sakamoto <roki@hongo.wide.ad.jp>");
MODULE_DESCRIPTION("PSEUDO SOCKET");
MODULE_LICENSE("GPL");
