function handle_click(event){
    const cell = event.target;
    const perm_attr = cell.attributes.getNamedItem("data-perm");
    if(perm_attr != null && perm_attr.value != ""){
        for(const other_cell of document.querySelectorAll("th." + perm_attr.value)){
            other_cell.classList.add("selected_elm");
        }
    }
}

function add_handlers(){
    for(const td of document.getElementsByTagName("td")){
        td.addEventListener("click",handle_click);
    }
    for(const th of document.getElementsByTagName("th")){
        th.addEventListener("click",handle_click);
    }
}

add_handlers();
