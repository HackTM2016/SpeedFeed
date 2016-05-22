#pragma once
#include <cpprest/http_client.h>
#include <cpprest/json.h>

typedef pplx::task<web::http::http_response> HTTP_RESPONSE;

class CRESTSender
{
public:
	CRESTSender(web::http::client::http_client & client);
	~CRESTSender();
	CRESTSender(CRESTSender const& rs)
		: m_httpClient(rs.m_httpClient)
	{
	}
	CRESTSender& operator = (CRESTSender const& rs)
	{
		if (this != &rs) {
			m_httpClient = rs.m_httpClient;
		}
		return *this;
	}

	HTTP_RESPONSE task_request(web::http::method mtd, 
                                            web::json::value const & jvalue);
	HTTP_RESPONSE task_string_request(web::http::method mtd, 
												   const utility::string_t &body_data);
 
	void make_request(web::http::method mtd, web::json::value const & jvalue);

private:
	web::http::client::http_client &m_httpClient;
};

