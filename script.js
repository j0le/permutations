function handle_click(event){
    const td = event.target;
    const perm_attr = td.attributes.getNamedItem("data-perm");
    if(perm_attr != null && perm_attr.value != ""){
        for(const other_td of document.getElementsByClassName(perm_attr.value)){
            other_td.classList.add("selected_elm");
        }
    }
}

function add_handlers(){
    for(const td of document.getElementsByTagName("td")){
        td.addEventListener("click",handle_click);
    }
}

add_handlers();
