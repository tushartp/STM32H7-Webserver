/*
 * libwebsockets - lib/plat/lws-plat-esp32.c
 *
 * Copyright (C) 2010-2017 Andy Green <andy@warmcat.com>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation:
 *  version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA  02110-1301  USA
 */

#include "core/private.h"

struct lws_stm32 lws_stm32 = {
	.model = "stm32",
	.serial = "h753",
	.le_email = "tcpatel@mircom.com",
	.hostname = "stm32-h753",
};




void
lws_stm32_set_creation_defaults(struct lws_context_creation_info *info)
{

	memset(info, 0, sizeof(*info));

	info->vhost_name = "default";
	info->port = 443;
	info->fd_limit_per_thread = 16;
	info->max_http_header_pool = 5;
	info->max_http_header_data = 1024;
	info->pt_serv_buf_size = 4096;
	info->keepalive_timeout = 30;
	info->timeout_secs = 30;
	info->simultaneous_ssl_restriction = 2;
	info->options = LWS_SERVER_OPTION_EXPLICIT_VHOSTS |
		        LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
}


static int
_rngf(void *context, unsigned char *buf, size_t len)
{
	if ((size_t)lws_get_random(context, buf, len) == len)
		return 0;

	return -1;
}

int
lws_esp32_selfsigned(struct lws_vhost *vhost)
{
	mbedtls_x509write_cert crt;
	char subject[200];
	mbedtls_pk_context mpk;
	int buf_size = 4096, n;
	uint8_t *buf = malloc(buf_size); /* malloc because given to user code */
	mbedtls_mpi mpi;
	uint8_t dummy[10];

	lwsl_notice("%s: %s\n", __func__, vhost->name);

	if (!buf)
		return -1;

	n=0;
	if(lws_plat_read_file(vhost->tls.alloc_cert_path, &dummy, 10)>0)
		n |= 1;
	if(lws_plat_read_file(vhost->tls.key_path, &dummy, 10)>0)
		n |= 2;

	if (n == 3) {
		lwsl_notice("%s: certs exist\n", __func__);
		free(buf);
		return 0; /* certs already exist */
	}


	lwsl_notice("%s: creating selfsigned initial certs\n", __func__);

	mbedtls_x509write_crt_init(&crt);

	mbedtls_pk_init(&mpk);
	if (mbedtls_pk_setup(&mpk, mbedtls_pk_info_from_type(MBEDTLS_PK_RSA))) {
		lwsl_notice("%s: pk_setup failed\n", __func__);
		goto fail;
	}
	lwsl_notice("%s: generating 2048-bit RSA keypair... "
		    "this may take a minute or so...\n", __func__);
	n = mbedtls_rsa_gen_key(mbedtls_pk_rsa(mpk), _rngf, vhost->context,
				2048, 65537);
	if (n) {
		lwsl_notice("%s: failed to generate keys\n", __func__);
		goto fail1;
	}
	lwsl_notice("%s: keys done\n", __func__);

	/* subject must be formatted like "C=TW,O=warmcat,CN=myserver" */

	lws_snprintf(subject, sizeof(subject) - 1,
		     "C=TW,ST=New Taipei City,L=Taipei,O=warmcat,CN=%s",
		     lws_stm32.hostname);

	if (mbedtls_x509write_crt_set_subject_name(&crt, subject)) {
		lwsl_notice("set SN failed\n");
		goto fail1;
	}
	mbedtls_x509write_crt_set_subject_key(&crt, &mpk);
	if (mbedtls_x509write_crt_set_issuer_name(&crt, subject)) {
		lwsl_notice("set IN failed\n");
		goto fail1;
	}
	mbedtls_x509write_crt_set_issuer_key(&crt, &mpk);

	lws_get_random(vhost->context, &n, sizeof(n));
	lws_snprintf(subject, sizeof(subject), "%d", n);

	mbedtls_mpi_init(&mpi);
	mbedtls_mpi_read_string(&mpi, 10, subject);
	mbedtls_x509write_crt_set_serial(&crt, &mpi);
	mbedtls_mpi_free(&mpi);

	mbedtls_x509write_crt_set_validity(&crt, "20171105235959",
					   "20491231235959");

	mbedtls_x509write_crt_set_key_usage(&crt,
					    MBEDTLS_X509_KU_DIGITAL_SIGNATURE |
					    MBEDTLS_X509_KU_KEY_ENCIPHERMENT);


	mbedtls_x509write_crt_set_md_alg(&crt, MBEDTLS_MD_SHA256);

	n = mbedtls_x509write_crt_pem(&crt, buf, buf_size, _rngf,
				      vhost->context);
	if (n < 0) {
		lwsl_notice("%s: write crt der failed\n", __func__);
		goto fail1;
	}

	lws_plat_write_cert(vhost, 0, 0, buf, strlen((const char *)buf));

	if (mbedtls_pk_write_key_pem(&mpk, buf, buf_size)) {
		lwsl_notice("write key pem failed\n");
		goto fail1;
	}

	lws_plat_write_cert(vhost, 1, 0, buf, strlen((const char *)buf));

	mbedtls_pk_free(&mpk);
	mbedtls_x509write_crt_free(&crt);

	lwsl_notice("%s: cert creation complete\n", __func__);
	free(buf);
	return n;

fail1:
	mbedtls_pk_free(&mpk);
fail:
	mbedtls_x509write_crt_free(&crt);
	free(buf);

	return -1;
}


struct lws_context *
lws_stm32_init(struct lws_context_creation_info *info, struct lws_vhost **pvh)
{

	struct lws_context *context;
	struct lws_vhost *vhost;
	struct lws wsi;

	context = lws_create_context(info);
	if (context == NULL) {
		lwsl_err("Failed to create context\n");
		return NULL;
	}

	info->options |= LWS_SERVER_OPTION_CREATE_VHOST_SSL_CTX |
			 LWS_SERVER_OPTION_IGNORE_MISSING_CERT;

	vhost = lws_create_vhost(context, info);
	if (!vhost) {
		lwsl_err("Failed to create vhost\n");
		return NULL;
	}

	lws_esp32_selfsigned(vhost);
	wsi.context = vhost->context;
	wsi.vhost = vhost;

	lws_tls_server_certs_load(vhost, &wsi, info->ssl_cert_filepath,
			info->ssl_private_key_filepath, NULL, 0, NULL, 0);

	lws_init_vhost_client_ssl(info, vhost);

	if (pvh)
		*pvh = vhost;

	if (lws_protocol_init(context))
		return NULL;

	return context;
}
