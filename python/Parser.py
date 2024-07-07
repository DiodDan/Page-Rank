import os
import requests
from bs4 import BeautifulSoup
from urllib.parse import urljoin, urlparse


def get_wikipedia_links(url):
    response = requests.get(url)
    soup = BeautifulSoup(response.content, 'html.parser')
    links = set()

    for a_tag in soup.find_all('a', href=True):
        href = a_tag['href']
        if href.startswith('/wiki/') and not href.startswith('/wiki/Special:') and not ':' in href:
            full_url = urljoin(url, href)
            links.add(full_url)

    return links


def save_html(url, directory):
    response = requests.get(url)
    soup = BeautifulSoup(response.content, 'html.parser')


    # Преобразуем URL в допустимое имя файла
    path = urlparse(url).path
    file_name = path.replace('/wiki/', '').replace('/', '_') + '.html'

    # Создаем директорию, если она не существует
    if not os.path.exists(directory):
        os.makedirs(directory)

    file_path = os.path.join(directory, file_name)
    with open(file_path, 'w', encoding='utf-8') as file:
        file.write(str(soup))

    return file_path

def crawl_wikipedia(start_url, directory, max_depth=5):
    visited = set()
    to_visit = {start_url}
    depth = 0

    while to_visit and depth < max_depth:
        current_to_visit = to_visit.copy()
        to_visit.clear()

        for url in current_to_visit:
            if url in visited:
                continue

            visited.add(url)
            print(f'Visiting: {url}')
            try:
                file_path = save_html(url, directory)
                print(f'Saved {url} as {file_path}')
            except Exception as e:
                print(f'Error saving {url}: {e}')
                continue

            links = get_wikipedia_links(url)
            to_visit.update(links)

        depth += 1

def main():
    start_url = 'https://en.wikipedia.org/wiki/Web_scraping'
    directory = 'wikipedia_pages'
    max_depth = 5

    crawl_wikipedia(start_url, directory, max_depth)


if __name__ == '__main__':
    main()
