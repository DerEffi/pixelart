export interface ISidebarItem {
    label: string;
    items: ISidebarSubItem[];
    icon?: JSX.Element;
    url?: string;
}

export interface ISidebarSubItem {
    label: string;
    icon?: JSX.Element;
    url: string;
}