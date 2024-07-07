import os
from bs4 import BeautifulSoup
from urllib.parse import urlparse, urljoin

def get_html_files(directory):
    return list({f for f in os.listdir(directory) if f.endswith('.html')})

def extract_links(html_content, base_url):
    soup = BeautifulSoup(html_content, 'html.parser')
    links = []

    for a_tag in soup.find_all('a', href=True):
        href = a_tag['href']
        parsed_href = urlparse(href)

        if parsed_href.netloc:
            continue

        full_url = urljoin(base_url, href)
        links.append(full_url)

    return links

def create_links_list(directory):
    html_files = get_html_files(directory)
    links_list = []

    for html_file in html_files:
        file_path = os.path.join(directory, html_file)
        with open(file_path, 'r', encoding='utf-8') as file:
            html_content = file.read()

        base_url = f'file://{os.path.abspath(file_path)}'
        links = extract_links(html_content, base_url)
        for link in links:
            file = link[13:]+".html"
            if  ('/wiki/') in link and file in html_files:
                links_list.append((html_file, link[13:]+".html"))

    return links_list

def main():
    directory = 'wikipedia_pages'
    links_list = create_links_list(directory)

    for link in links_list:
        print(link)

if __name__ == '__main__':
    main()

