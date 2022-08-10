/*
 * Copyright (C) Internet Systems Consortium, Inc. ("ISC")
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * See the COPYRIGHT file distributed with this work for additional
 * information regarding copyright ownership.
 */

#ifndef WIN32
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif /* ifndef WIN32 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <isc/base64.h>
#include <isc/buffer.h>
#include <isc/commandline.h>
#include <isc/lib.h>
#include <isc/mem.h>
#include <isc/print.h>
#include <isc/sockaddr.h>
#include <isc/util.h>

#include <dns/client.h>
#include <dns/fixedname.h>
#include <dns/keyvalues.h>
#include <dns/lib.h>
#include <dns/masterdump.h>
#include <dns/message.h>
#include <dns/name.h>
#include <dns/rdata.h>
#include <dns/rdataset.h>
#include <dns/rdatastruct.h>
#include <dns/rdatatype.h>
#include <dns/result.h>
#include <dns/secalg.h>

#include <dst/dst.h>

static isc_mem_t *mctx;
static dns_fixedname_t fixedqname;

ISC_PLATFORM_NORETURN_PRE static void
usage(void) ISC_PLATFORM_NORETURN_POST;

static void
usage(void) {
	fprintf(stderr, "sample-request [-t RRtype] server_address hostname\n");

	exit(1);
}

static isc_result_t
make_querymessage(dns_message_t *message, const char *namestr,
		  dns_rdatatype_t rdtype) {
	dns_name_t *qname = NULL, *qname0;
	dns_rdataset_t *qrdataset = NULL;
	isc_result_t result;
	isc_buffer_t b;
	unsigned int namelen;

	REQUIRE(message != NULL);
	REQUIRE(namestr != NULL);

	/* Construct qname */
	namelen = strlen(namestr);
	isc_buffer_constinit(&b, namestr, namelen);
	isc_buffer_add(&b, namelen);
	qname0 = dns_fixedname_initname(&fixedqname);
	result = dns_name_fromtext(qname0, &b, dns_rootname, 0, NULL);
	if (result != ISC_R_SUCCESS) {
		fprintf(stderr, "failed to convert qname: %u\n", result);
		return (result);
	}

	/* Construct query message */
	message->opcode = dns_opcode_query;
	message->rdclass = dns_rdataclass_in;

	result = dns_message_gettempname(message, &qname);
	if (result != ISC_R_SUCCESS) {
		goto cleanup;
	}

	result = dns_message_gettemprdataset(message, &qrdataset);
	if (result != ISC_R_SUCCESS) {
		goto cleanup;
	}

	dns_name_init(qname, NULL);
	dns_name_clone(qname0, qname);
	dns_rdataset_makequestion(qrdataset, message->rdclass, rdtype);
	ISC_LIST_APPEND(qname->list, qrdataset, link);
	dns_message_addname(message, qname, DNS_SECTION_QUESTION);

	return (ISC_R_SUCCESS);

cleanup:
	if (qname != NULL) {
		dns_message_puttempname(message, &qname);
	}
	if (qrdataset != NULL) {
		dns_message_puttemprdataset(message, &qrdataset);
	}
	dns_message_detach(&message);
	return (result);
}

static void
print_section(dns_message_t *message, int section, isc_buffer_t *buf) {
	isc_result_t result;
	isc_region_t r;

	result = dns_message_sectiontotext(message, section,
					   &dns_master_style_full, 0, buf);
	if (result != ISC_R_SUCCESS) {
		goto fail;
	}

	isc_buffer_usedregion(buf, &r);
	printf("%.*s", (int)r.length, (char *)r.base);

	return;

fail:
	fprintf(stderr, "failed to convert a section\n");
}

int
main(int argc, char *argv[]) {
	int ch, i, gaierror;
	struct addrinfo hints, *res;
	isc_textregion_t tr;
	dns_client_t *client = NULL;
	isc_result_t result;
	isc_sockaddr_t sa;
	dns_message_t *qmessage, *rmessage;
	dns_rdatatype_t type = dns_rdatatype_a;
	isc_buffer_t *outputbuf;

	while ((ch = isc_commandline_parse(argc, argv, "t:")) != -1) {
		switch (ch) {
		case 't':
			tr.base = isc_commandline_argument;
			tr.length = strlen(isc_commandline_argument);
			result = dns_rdatatype_fromtext(&type, &tr);
			if (result != ISC_R_SUCCESS) {
				fprintf(stderr, "invalid RRtype: %s\n",
					isc_commandline_argument);
				exit(1);
			}
			break;
		default:
			usage();
		}
	}

	argc -= isc_commandline_index;
	argv += isc_commandline_index;
	if (argc < 2) {
		usage();
	}

	isc_lib_register();
	result = dns_lib_init();
	if (result != ISC_R_SUCCESS) {
		fprintf(stderr, "dns_lib_init failed: %u\n", result);
		exit(1);
	}

	result = dns_client_create(&client, 0);
	if (result != ISC_R_SUCCESS) {
		fprintf(stderr, "dns_client_create failed: %u\n", result);
		exit(1);
	}

	/* Prepare message structures */
	mctx = NULL;
	qmessage = NULL;
	rmessage = NULL;

	isc_mem_create(&mctx);
	dns_message_create(mctx, DNS_MESSAGE_INTENTRENDER, &qmessage);
	dns_message_create(mctx, DNS_MESSAGE_INTENTPARSE, &rmessage);

	/* Initialize the nameserver address */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
#ifdef AI_NUMERICHOST
	hints.ai_flags = AI_NUMERICHOST;
#endif /* ifdef AI_NUMERICHOST */
	gaierror = getaddrinfo(argv[0], "53", &hints, &res);
	if (gaierror != 0) {
		fprintf(stderr, "getaddrinfo failed: %s\n",
			gai_strerror(gaierror));
		exit(1);
	}
	INSIST(res->ai_addrlen <= sizeof(sa.type));
	memmove(&sa.type, res->ai_addr, res->ai_addrlen);
	freeaddrinfo(res);
	sa.length = (unsigned int)res->ai_addrlen;
	ISC_LINK_INIT(&sa, link);

	/* Construct qname */
	result = make_querymessage(qmessage, argv[1], type);
	if (result != ISC_R_SUCCESS) {
		fprintf(stderr, "failed to create a query\n");
		exit(1);
	}

	/* Send request and wait for a response */
	result = dns_client_request(client, qmessage, rmessage, &sa, 0, 0, NULL,
				    60, 0, 3);
	if (result != ISC_R_SUCCESS) {
		fprintf(stderr, "failed to get a response: %s\n",
			dns_result_totext(result));
	}

	/* Dump the response */
	outputbuf = NULL;
	isc_buffer_allocate(mctx, &outputbuf, 65535);
	for (i = 0; i < DNS_SECTION_MAX; i++) {
		print_section(rmessage, i, outputbuf);
		isc_buffer_clear(outputbuf);
	}
	isc_buffer_free(&outputbuf);

	/* Cleanup */
	dns_message_detach(&qmessage);
	dns_message_detach(&rmessage);
	isc_mem_destroy(&mctx);
	dns_client_destroy(&client);
	dns_lib_shutdown();

	return (0);
}