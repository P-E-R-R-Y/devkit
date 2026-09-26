/**
 * @file Http.hpp
 * @brief Une requete HTTPS, par libcurl.
 *
 * Liee, pas appelee : devkit ne depend d'aucun binaire installe chez
 * l'utilisateur. Le C++ n'a pas de client HTTP dans sa bibliotheque
 * standard, et TLS impose de toute facon une dependance.
 */

#pragma once

#include <curl/curl.h>

#include <string>

namespace http {

    struct Answer {
        long status = 0;        ///< 0 : la requete n'est meme pas partie
        std::string body;

        bool ok() const { return status >= 200 && status < 300; }
    };

    namespace detail {

        inline std::size_t collect(char *data, std::size_t size, std::size_t count, void *out) {
            static_cast<std::string *>(out)->append(data, size * count);
            return size * count;
        }
    }

    /**
     * @brief GET, ou POST si `payload` n'est pas vide.
     *
     * @param token jeton GitHub, facultatif : il leve la limite de 60
     *              requetes par heure et ouvre l'acces a GraphQL
     */
    inline Answer request(const std::string &url, const std::string &token = "",
                          const std::string &payload = "") {
        Answer answer;
        CURL *curl = curl_easy_init();

        if (!curl)
            return answer;

        curl_slist *headers = nullptr;

        headers = curl_slist_append(headers, "Accept: application/vnd.github+json");
        if (!token.empty())
            headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        //github refuse les requetes sans agent
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "devkit");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, detail::collect);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &answer.body);
        if (!payload.empty())
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());

        if (curl_easy_perform(curl) == CURLE_OK)
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &answer.status);

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return answer;
    }
}
