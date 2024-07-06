import os
import requests
from bs4 import BeautifulSoup
from urllib.parse import urljoin, urlparse


def get_wikipedia_links(url):
    response = requests.get(url)
    soup = BeautifulSoup(response.content, 'html.parser')
    links = []

    for a_tag in soup.find_all('a', href=True):
        href = a_tag['href']
        if href.startswith('/wiki/') and not href.startswith('/wiki/Special:'):
            full_url = urljoin(url, href)
            links.append(full_url)

    return links


def save_html(url, directory):
    response = requests.get(url)
    soup = BeautifulSoup(response.content, 'html.parser')

    path = urlparse(url).path
    file_name = path.replace('/wiki/', "").replace('/', '_') + '.html'

    if not os.path.exists(directory):
        os.makedirs(directory)

    file_path = os.path.join(directory, file_name)
    with open(file_path, 'w', encoding='utf-8') as file:
        file.write(str(soup))

    return file_path


def main():
    start_url = 'https://en.wikipedia.org/wiki/Main_Page'
    directory = 'wikipedia_pages'

    links = get_wikipedia_links(start_url)

    print(f'Found {len(links)} links')

    for link in links:
        file_path = save_html(link, directory)
        print(f'Saved {link} as {file_path}')


if __name__ == '__main__':
    main()
