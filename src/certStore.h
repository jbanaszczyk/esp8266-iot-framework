//Since findHashedTA in BearSSL::CertStore is a non-virtual member
//the class is completely overwritten by using the same preprocessor
//include guard

#ifndef _CERTSTORE_BEARSSL_H
#define _CERTSTORE_BEARSSL_H

#include <Arduino.h>
#include <BearSSLHelpers.h>
#include <bearssl/bearssl.h>

namespace BearSSL {

	class CertStore {
	public:
		CertStore() {};

		// Installs the cert store into the X509 decoder (normally via static function callbacks)
		void installCertStore(br_x509_minimal_context *context);

	protected:
		X509List *_x509 = nullptr;

		// These need to be static as they are callbacks from BearSSL C code
		static const br_x509_trust_anchor *findHashedTA(void *context, void *hashed_dn, size_t length);

		static void freeHashedTA(void *ctx, const br_x509_trust_anchor *ta);
	};
};

#endif
