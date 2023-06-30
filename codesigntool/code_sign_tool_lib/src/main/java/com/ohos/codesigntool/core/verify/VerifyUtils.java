/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.ohos.codesigntool.core.verify;

import org.bouncycastle.cert.X509CertificateHolder;
import org.bouncycastle.cms.CMSException;
import org.bouncycastle.cms.CMSProcessableByteArray;
import org.bouncycastle.cms.CMSSignedData;
import org.bouncycastle.cms.jcajce.JcaSimpleSignerInfoVerifierBuilder;
import org.bouncycastle.jce.provider.BouncyCastleProvider;
import org.bouncycastle.operator.OperatorCreationException;
import org.bouncycastle.util.Store;

import java.security.Provider;
import java.security.Security;
import java.security.cert.CertificateException;
import java.util.Collection;

/**
 * Utils for verify CMS signed data
 *
 * @since 2023/06/05
 */
public class VerifyUtils {
    /**
     * Constructor of Method
     */
    private VerifyUtils() {
    }

    static {
        Provider bc = Security.getProvider("BC");
        if (bc == null) {
            Security.addProvider(new BouncyCastleProvider());
        }
    }

    /**
     * Verify cms signed data
     *
     * @param cmsSignedData cms signed data
     * @return true, if verify success
     * @throws CMSException if error occurs
     */
    @SuppressWarnings("unchecked")
    public static boolean verifyCmsSignedData(CMSSignedData cmsSignedData) throws CMSException {
        Store<X509CertificateHolder> certs = cmsSignedData.getCertificates();
        boolean verifyResult = cmsSignedData.verifySignatures(sid -> {
            try {
                Collection<X509CertificateHolder> collection = certs.getMatches(sid);
                if ((collection == null) || (collection.size() != 1)) {
                    throw new OperatorCreationException(
                        "No matched cert or more than one matched certs: " + collection);
                }
                X509CertificateHolder cert = collection.iterator().next();
                return new JcaSimpleSignerInfoVerifierBuilder().setProvider("BC").build(cert);
            } catch (CertificateException e) {
                throw new OperatorCreationException("Failed to verify BC signatures: " + e.getMessage(), e);
            }
        });
        return verifyResult;
    }

    /**
     * Verify cms signed data which has no content
     *
     * @param contentData signed content data
     * @param pkcs7 cms signed data
     * @return true if verify success
     * @throws CMSException if error
     */
    public static boolean verifyPKCS7withContent(byte[] contentData, byte[] pkcs7) throws CMSException {
        CMSSignedData cmsSignedData = new CMSSignedData(new CMSProcessableByteArray(contentData), pkcs7);
        return VerifyUtils.verifyCmsSignedData(cmsSignedData);
    }
}
