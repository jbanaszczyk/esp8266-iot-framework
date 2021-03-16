#include "CertStore.h"
#include "generated/certificates.h"

#if defined(DEBUG_IOT_CERT_STORE) && defined(DEBUG_IOT_PORT)
#define LOG_CERT(...) DEBUG_IOT_PORT.printf_P( "[CERT] " __VA_ARGS__ )
#else
#define LOG_CERT(...)
#endif

namespace BearSSL {
	void CertStore::installCertStore(br_x509_minimal_context *context) {
		br_x509_minimal_set_dynamic(context, (void *) this, findHashedTA, freeHashedTA);
	}

	const br_x509_trust_anchor *CertStore::findHashedTA(void *context, void *hashed_dn, size_t length) {

		//compare sha256 from index file with hashed_dn
		//then return certificate
		auto *pCertStore = static_cast<CertStore *>(context);

		if (!pCertStore || length != 32) {
			return nullptr;
		}

		for (uint16_t certIndex = 0; certIndex < numberOfCertificates; ++certIndex) {
			if (!memcmp_P(hashed_dn, indices[certIndex], 32)) {
				LOG_CERT("Certificate found at index %d\n", certIndex);

				uint16_t certSize[1];
				memcpy_P(certSize, certSizes + certIndex, 2);

				auto *der = (uint8_t *) malloc(certSize[0]);
				memcpy_P(der, certificates[certIndex], certSize[0]);
				pCertStore->_x509 = new X509List(der, certSize[0]);
				free(der);

				if (!pCertStore->_x509) {
					return nullptr;
				}

				auto *trust_anchor = (br_x509_trust_anchor *) pCertStore->_x509->getTrustAnchors();
				memcpy_P(trust_anchor->dn.data, indices[certIndex], 32);
				trust_anchor->dn.len = 32;

				return trust_anchor;
			}
		}
		LOG_CERT("Certificate not found\n");
		return nullptr;
	}

	void CertStore::freeHashedTA(void *ctx, const br_x509_trust_anchor *ta) {
		auto *pCertStore = static_cast<CertStore *>(ctx);
		(void) ta; // Unused
		delete pCertStore->_x509;
		pCertStore->_x509 = nullptr;
	}
}
